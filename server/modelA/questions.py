from flask import Blueprint, request, jsonify
import time
from database.init import user_exists

questions_bp = Blueprint("questions", __name__)


def get_db():
    from database.init import get_db_path, init_db
    import sqlite3
    db_path = get_db_path()
    init_db()
    conn = sqlite3.connect(db_path)
    conn.row_factory = sqlite3.Row
    return conn


# ──────────────────────────────────────────
# 获取问答列表
# ──────────────────────────────────────────

@questions_bp.route("/questions", methods=["GET"])
def get_questions():
    """
    获取问答列表
    URL 参数: ?class=xxx&course=xxx&sort=hot|new
    返回: 帖子列表，含回复的 reply_to_username
    """
    class_val = request.args.get("class", type=int)
    course = request.args.get("course", "").strip()
    sort = request.args.get("sort", "new").strip()

    conn = get_db()

    # 获取所有主贴 (parent_id = 0)
    query = "SELECT * FROM questions WHERE parent_id = 0"
    params = []

    if class_val:
        query += " AND class = ?"
        params.append(class_val)
    if course:
        query += " AND course = ?"
        params.append(course)

    if sort == "hot":
        query += " ORDER BY like_count DESC, time DESC"
    else:
        query += " ORDER BY time DESC"

    posts = conn.execute(query, params).fetchall()

    result = []
    for post in posts:
        # 获取该帖子的所有回复
        replies = conn.execute(
            "SELECT * FROM questions WHERE parent_id = ? AND id != ? ORDER BY time ASC",
            (post["post_id"], post["id"]),
        ).fetchall()

        reply_list = []
        for reply in replies:
            # 查出回复的 parent post uploader（即被回复者的用户名）
            reply_to = conn.execute(
                "SELECT uploader FROM questions WHERE post_id = ? LIMIT 1",
                (reply["parent_id"],),
            ).fetchone()

            reply_list.append({
                "id": reply["id"],
                "post_id": reply["post_id"],
                "parent_id": reply["parent_id"],
                "uploader": reply["uploader"],
                "content": reply["content"],
                "time": reply["time"],
                "like_count": reply["like_count"],
                "reply_to_username": reply_to["uploader"] if reply_to else "",
            })

        result.append({
            "id": post["id"],
            "post_id": post["post_id"],
            "uploader": post["uploader"],
            "content": post["content"],
            "time": post["time"],
            "course": post["course"],
            "like_count": post["like_count"],
            "replies": reply_list,
        })

    conn.close()
    return jsonify({"success": True, "data": result}), 200


# ──────────────────────────────────────────
# 发布问答 / 回复
# ──────────────────────────────────────────

@questions_bp.route("/questions/publish", methods=["POST"])
def publish_question():
    """发布新帖或回复"""
    data = request.get_json(silent=True)
    if not data:
        return jsonify({"success": False, "message": "请求体为空"}), 400

    uploader = data.get("uploader", "").strip()
    course = data.get("course", "").strip()
    content = data.get("content", "").strip()
    parent_id = data.get("parent_id", 0)

    if not uploader or not content:
        return jsonify({"success": False, "message": "缺少必要参数"}), 400

    class_val = data.get("class", 0)
    try:
        class_val = int(class_val)
    except (TypeError, ValueError):
        class_val = 0

    try:
        parent_id = int(parent_id)
    except (TypeError, ValueError):
        parent_id = 0

    # 校验用户是否存在（防止已删账号继续操作）
    if not user_exists(uploader):
        return jsonify({"success": False, "message": "用户不存在"}), 404

    conn = get_db()

    # 生成 post_id
    max_post = conn.execute(
        "SELECT MAX(post_id) FROM questions"
    ).fetchone()[0]
    new_post_id = (max_post or 0) + 1

    now = time.strftime("%Y/%m/%d")
    conn.execute(
        """INSERT INTO questions (uploader, post_id, parent_id, class, time, course, content)
           VALUES (?, ?, ?, ?, ?, ?, ?)""",
        (uploader, new_post_id, parent_id, class_val, now, course, content),
    )
    conn.commit()

    # 如果回复，返回 reply_to_username
    reply_to_username = ""
    if parent_id > 0:
        parent = conn.execute(
            "SELECT uploader FROM questions WHERE post_id = ? LIMIT 1",
            (parent_id,),
        ).fetchone()
        if parent:
            reply_to_username = parent["uploader"]

    conn.close()

    return jsonify({
        "success": True,
        "message": "发布成功",
        "post_id": new_post_id,
        "reply_to_username": reply_to_username,
    }), 201


# ──────────────────────────────────────────
# 点赞
# ──────────────────────────────────────────

@questions_bp.route("/questions/like", methods=["POST"])
def like_question():
    """切换点赞/取消点赞"""
    data = request.get_json(silent=True)
    if not data:
        return jsonify({"success": False, "message": "请求体为空"}), 400

    post_id = data.get("post_id", 0)
    username = data.get("username", "").strip()
    try:
        post_id = int(post_id)
    except (TypeError, ValueError):
        post_id = 0

    if post_id <= 0 or not username:
        return jsonify({"success": False, "message": "缺少必要的参数"}), 400

    if not user_exists(username):
        return jsonify({"success": False, "message": "用户不存在"}), 404

    conn = get_db()

    # 检查是否已点赞
    existing = conn.execute(
        "SELECT id FROM question_likes WHERE post_id = ? AND username = ?",
        (post_id, username),
    ).fetchone()

    if existing:
        # 取消点赞
        conn.execute("DELETE FROM question_likes WHERE id = ?", (existing["id"],))
        conn.execute("UPDATE questions SET like_count = MAX(0, like_count - 1) WHERE post_id = ?", (post_id,))
        liked = False
    else:
        # 点赞
        conn.execute("INSERT OR IGNORE INTO question_likes (post_id, username) VALUES (?, ?)", (post_id, username))
        conn.execute("UPDATE questions SET like_count = like_count + 1 WHERE post_id = ?", (post_id,))
        liked = True

    conn.commit()
    updated = conn.execute(
        "SELECT like_count FROM questions WHERE post_id = ?", (post_id,)
    ).fetchone()
    conn.close()

    if not updated:
        return jsonify({"success": False, "message": "帖子不存在"}), 404

    return jsonify({
        "success": True,
        "liked": liked,
        "like_count": updated["like_count"],
    }), 200
