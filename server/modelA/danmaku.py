from flask import Blueprint, request, jsonify
from database.init import user_exists

danmaku_bp = Blueprint("danmaku", __name__)


def get_db():
    from database.init import get_db_path, init_db
    import sqlite3
    db_path = get_db_path()
    init_db()
    conn = sqlite3.connect(db_path)
    conn.row_factory = sqlite3.Row
    return conn


@danmaku_bp.route("/danmaku/send", methods=["POST"])
def send_danmaku():
    """
    发送弹幕
    请求体: {video_id, sender, content, play_time, class}
    """
    data = request.get_json(silent=True)
    if not data:
        return jsonify({"success": False, "message": "请求体为空"}), 400

    video_id = data.get("video_id")
    sender = data.get("sender", "").strip()
    content = data.get("content", "").strip()
    play_time = data.get("play_time")
    class_val = data.get("class")

    if not video_id or not sender or not content:
        return jsonify({"success": False, "message": "缺少必要参数"}), 400

    if len(content) > 50:
        return jsonify({"success": False, "message": "弹幕内容不能超过50个字符"}), 400

    if not user_exists(sender):
        return jsonify({"success": False, "message": "用户不存在"}), 404

    try:
        video_id = int(video_id)
        play_time = int(play_time) if play_time is not None else 0
        class_val = int(class_val) if class_val is not None else 0
    except (TypeError, ValueError):
        return jsonify({"success": False, "message": "参数类型错误"}), 400

    conn = get_db()
    cursor = conn.execute(
        """INSERT INTO danmaku (video_id, sender, content, play_time, class)
           VALUES (?, ?, ?, ?, ?)""",
        (video_id, sender, content, play_time, class_val),
    )
    danmaku_id = cursor.lastrowid
    conn.commit()
    conn.close()

    return jsonify({
        "success": True,
        "message": "弹幕发送成功",
        "id": danmaku_id,
    }), 201


@danmaku_bp.route("/danmaku/init", methods=["GET"])
def init_danmaku():
    """
    获取某视频的所有弹幕
    URL 参数: ?video_id=xxx
    """
    video_id = request.args.get("video_id", type=int)
    if not video_id:
        return jsonify({"success": False, "message": "缺少 video_id 参数"}), 400

    conn = get_db()
    rows = conn.execute(
        """SELECT id, video_id, sender, content, play_time, create_time
           FROM danmaku WHERE video_id = ? ORDER BY id""",
        (video_id,),
    ).fetchall()
    conn.close()

    return jsonify({
        "success": True,
        "data": [dict(row) for row in rows],
    }), 200


@danmaku_bp.route("/danmaku/poll", methods=["GET"])
def poll_danmaku():
    """
    轮询新弹幕（增量获取）
    URL 参数: ?video_id=xxx&last_id=xxx
    """
    video_id = request.args.get("video_id", type=int)
    last_id = request.args.get("last_id", type=int)

    if not video_id or last_id is None:
        return jsonify({"success": False, "message": "缺少 video_id 或 last_id 参数"}), 400

    conn = get_db()
    rows = conn.execute(
        """SELECT id, video_id, sender, content, play_time, create_time
           FROM danmaku WHERE video_id = ? AND id > ? ORDER BY id""",
        (video_id, last_id),
    ).fetchall()
    conn.close()

    return jsonify({
        "success": True,
        "data": [dict(row) for row in rows],
    }), 200
