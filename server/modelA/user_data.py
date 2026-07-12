from flask import Blueprint, request, jsonify
import time
from database.init import user_exists

user_bp = Blueprint("user_data", __name__)


def get_db():
    from database.init import get_db_path, init_db
    import sqlite3
    db_path = get_db_path()
    init_db()
    conn = sqlite3.connect(db_path)
    conn.row_factory = sqlite3.Row
    return conn


def get_username_from_request():
    """从请求中获取用户名（当前简化：从查询参数或JSON体获取）"""
    if request.method == "GET":
        return request.args.get("username", "").strip()
    data = request.get_json(silent=True) or {}
    return data.get("username", request.args.get("username", "")).strip()


# ──────────────────────────────────────────
# 浏览历史
# ──────────────────────────────────────────

@user_bp.route("/history", methods=["GET"])
def get_history():
    """
    获取浏览历史
    URL 参数: ?username=xxx&days=N（days为空则返回全部）
    """
    username = request.args.get("username", "").strip()
    days = request.args.get("days", type=int)

    if not username:
        return jsonify({"success": False, "message": "缺少用户名参数"}), 400

    conn = get_db()
    if days:
        # 最近 N 天
        seconds = days * 86400
        cutoff = int(time.time()) - seconds
        rows = conn.execute(
            """SELECT id, video_id, video_title, teacher, class, view_time
               FROM view_history
               WHERE username = ? AND CAST(strftime('%s', view_time) AS INTEGER) >= ?
               ORDER BY view_time DESC
               LIMIT 50""",
            (username, cutoff),
        ).fetchall()
    else:
        rows = conn.execute(
            """SELECT id, video_id, video_title, teacher, class, view_time
               FROM view_history
               WHERE username = ?
               ORDER BY view_time DESC
               LIMIT 100""",
            (username,),
        ).fetchall()

    conn.close()

    return jsonify({
        "success": True,
        "data": [dict(row) for row in rows],
    }), 200


@user_bp.route("/history", methods=["POST"])
def add_history():
    """记录浏览"""
    data = request.get_json(silent=True)
    if not data:
        return jsonify({"success": False, "message": "请求体为空"}), 400

    username = data.get("username", "").strip()
    video_id = data.get("video_id")
    if video_id is not None:
        video_id = int(video_id)
    video_title = data.get("video_title", "").strip()
    teacher = data.get("teacher", "").strip()
    class_val = data.get("class")
    if class_val is not None:
        class_val = int(class_val)

    if not username or not video_id:
        return jsonify({"success": False, "message": "缺少必要参数"}), 400

    if not user_exists(username):
        return jsonify({"success": False, "message": "用户不存在"}), 404

    conn = get_db()
    conn.execute(
        """INSERT INTO view_history (username, video_id, video_title, teacher, class)
           VALUES (?, ?, ?, ?, ?)""",
        (username, video_id, video_title, teacher, class_val),
    )
    conn.commit()
    conn.close()

    return jsonify({"success": True, "message": "记录成功"}), 201


# ──────────────────────────────────────────
# 下载记录
# ──────────────────────────────────────────

@user_bp.route("/downloads", methods=["GET"])
def get_downloads():
    """获取下载记录"""
    username = request.args.get("username", "").strip()

    if not username:
        return jsonify({"success": False, "message": "缺少用户名参数"}), 400

    conn = get_db()
    rows = conn.execute(
        """SELECT id, file_name, file_type, file_size, class, download_time
           FROM download_history
           WHERE username = ?
           ORDER BY download_time DESC
           LIMIT 50""",
        (username,),
    ).fetchall()
    conn.close()

    return jsonify({
        "success": True,
        "data": [dict(row) for row in rows],
    }), 200


@user_bp.route("/downloads", methods=["POST"])
def add_download():
    """记录下载"""
    data = request.get_json(silent=True)
    if not data:
        return jsonify({"success": False, "message": "请求体为空"}), 400

    username = data.get("username", "").strip()
    file_name = data.get("file_name", "").strip()
    file_type = data.get("file_type", "").strip()
    file_size = data.get("file_size")
    if file_size is not None:
        file_size = int(file_size)
    class_val = data.get("class")
    if class_val is not None:
        class_val = int(class_val)

    if not username or not file_name:
        return jsonify({"success": False, "message": "缺少必要参数"}), 400

    if not user_exists(username):
        return jsonify({"success": False, "message": "用户不存在"}), 404

    conn = get_db()
    conn.execute(
        """INSERT INTO download_history (username, file_name, file_type, file_size, class)
           VALUES (?, ?, ?, ?, ?)""",
        (username, file_name, file_type, file_size, class_val),
    )
    conn.commit()
    conn.close()

    return jsonify({"success": True, "message": "记录成功"}), 201


# ──────────────────────────────────────────
# 用户上传列表
# ──────────────────────────────────────────

@user_bp.route("/uploads", methods=["GET"])
def get_uploads():
    """获取用户上传的内容（视频 + 资源）"""
    username = request.args.get("username", "").strip()

    if not username:
        return jsonify({"success": False, "message": "缺少用户名参数"}), 400

    conn = get_db()

    # 查询用户上传的视频（教师身份）
    videos = conn.execute(
        """SELECT id, course AS title, time, class, 'video' AS item_type
           FROM courses
           WHERE teacher = ?
           ORDER BY time DESC
           LIMIT 50""",
        (username,),
    ).fetchall()

    # 查询用户上传的资源
    resources = conn.execute(
        """SELECT id, name AS title, time, class, 'resource' AS item_type
           FROM resources
           WHERE uploader = ?
           ORDER BY time DESC
           LIMIT 50""",
        (username,),
    ).fetchall()

    conn.close()

    items = [dict(r) for r in videos] + [dict(r) for r in resources]
    # 按时间倒序排列
    items.sort(key=lambda x: x.get("time", ""), reverse=True)

    return jsonify({"success": True, "data": items}), 200


# ──────────────────────────────────────────
# 收藏夹
# ──────────────────────────────────────────

@user_bp.route("/favorites", methods=["GET"])
def get_favorites():
    """获取收藏列表"""
    username = request.args.get("username", "").strip()

    if not username:
        return jsonify({"success": False, "message": "缺少用户名参数"}), 400

    conn = get_db()
    rows = conn.execute(
        """SELECT f.id, f.item_type, f.item_id, f.item_title, f.class, f.added_time,
                  c.subject, c.function, c.teacher, c.time AS course_time
           FROM favorites f
           LEFT JOIN courses c ON f.item_type = 'video' AND f.item_id = c.id
           WHERE f.username = ?
           ORDER BY f.added_time DESC
           LIMIT 100""",
        (username,),
    ).fetchall()
    conn.close()

    data = []
    for row in rows:
        d = dict(row)
        if d["item_type"] == "resource":
            d["subject"] = ""
            d["function"] = ""
            d["teacher"] = ""
            d["course_time"] = ""
        else:
            d["subject"] = d["subject"] or ""
            d["function"] = d["function"] or ""
            d["teacher"] = d["teacher"] or ""
            d["course_time"] = d["course_time"] or ""
        data.append(d)

    return jsonify({
        "success": True,
        "data": data,
    }), 200


@user_bp.route("/favorites", methods=["POST"])
def add_favorite():
    """添加收藏"""
    data = request.get_json(silent=True)
    if not data:
        return jsonify({"success": False, "message": "请求体为空"}), 400

    username = data.get("username", "").strip()
    item_type = data.get("item_type", "").strip()
    item_id = data.get("item_id")
    if item_id is not None:
        item_id = int(item_id)
    item_title = data.get("item_title", "").strip()
    class_val = data.get("class")
    if class_val is not None:
        class_val = int(class_val)

    if not username or not item_type or not item_id:
        return jsonify({"success": False, "message": "缺少必要参数"}), 400
    if item_type not in ("video", "resource"):
        return jsonify({"success": False, "message": "item_type 必须为 video 或 resource"}), 400

    if not user_exists(username):
        return jsonify({"success": False, "message": "用户不存在"}), 404

    conn = get_db()
    # 检查是否已收藏
    existing = conn.execute(
        "SELECT id FROM favorites WHERE username = ? AND item_type = ? AND item_id = ?",
        (username, item_type, item_id),
    ).fetchone()

    if existing:
        conn.close()
        return jsonify({"success": False, "message": "已收藏过该内容"}), 409

    conn.execute(
        """INSERT INTO favorites (username, item_type, item_id, item_title, class)
           VALUES (?, ?, ?, ?, ?)""",
        (username, item_type, item_id, item_title, class_val),
    )
    conn.commit()
    new_id = conn.execute("SELECT last_insert_rowid()").fetchone()[0]
    conn.close()

    return jsonify({"success": True, "message": "收藏成功", "favorite_id": new_id}), 201


@user_bp.route("/favorites/<int:fav_id>", methods=["DELETE"])
def remove_favorite(fav_id):
    """取消收藏"""
    username = request.args.get("username", "").strip()

    conn = get_db()
    # 验证所有权
    fav = conn.execute(
        "SELECT id FROM favorites WHERE id = ? AND username = ?",
        (fav_id, username),
    ).fetchone()

    if not fav:
        conn.close()
        return jsonify({"success": False, "message": "收藏不存在"}), 404

    conn.execute("DELETE FROM favorites WHERE id = ?", (fav_id,))
    conn.commit()
    conn.close()

    return jsonify({"success": True, "message": "已取消收藏"}), 200
