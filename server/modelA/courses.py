from flask import Blueprint, request, jsonify

courses_bp = Blueprint("courses", __name__)


def get_db():
    from database.init import get_db_path, init_db
    import sqlite3
    db_path = get_db_path()
    init_db()
    conn = sqlite3.connect(db_path)
    conn.row_factory = sqlite3.Row
    return conn


@courses_bp.route("/courses", methods=["GET"])
def get_courses():
    """
    获取课程视频列表
    URL 参数: ?class=xxx&tag=xxx&teacher=xxx&keyword=xxx
    班级参数自动从登录信息获取（当前简化：通过查询参数传入）
    """
    class_val = request.args.get("class", type=int)
    tag = request.args.get("tag", "").strip()
    teacher = request.args.get("teacher", "").strip()
    keyword = request.args.get("keyword", "").strip()

    if not class_val:
        return jsonify({"success": False, "message": "缺少班级参数"}), 400

    conn = get_db()
    query = "SELECT id, course, teacher, time, file_path, class FROM courses WHERE class = ?"
    params = [class_val]

    if teacher:
        query += " AND teacher LIKE ?"
        params.append(f"%{teacher}%")
    if keyword:
        query += " AND (course LIKE ? OR file_path LIKE ?)"
        params.append(f"%{keyword}%")
        params.append(f"%{keyword}%")

    query += " ORDER BY time DESC"

    rows = conn.execute(query, params).fetchall()
    conn.close()

    courses_list = [
        {
            "id": row["id"],
            "course": row["course"],
            "teacher": row["teacher"],
            "time": row["time"],
            "file_path": row["file_path"],
            "class": row["class"],
        }
        for row in rows
    ]

    return jsonify({"success": True, "data": courses_list}), 200
