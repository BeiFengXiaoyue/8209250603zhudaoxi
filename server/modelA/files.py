import os
from flask import Blueprint, send_file, jsonify, abort

files_bp = Blueprint("files", __name__)

UPLOAD_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "uploads", "courses")


def get_db():
    from database.init import get_db_path, init_db
    import sqlite3
    db_path = get_db_path()
    init_db()
    conn = sqlite3.connect(db_path)
    conn.row_factory = sqlite3.Row
    return conn


@files_bp.route("/courses/<int:course_id>/download", methods=["GET"])
def download_course(course_id):
    """
    下载课程视频文件
    URL: GET /api/courses/<course_id>/download
    返回: 视频文件（附件下载）
    """
    conn = get_db()
    row = conn.execute(
        "SELECT course, file_path FROM courses WHERE id = ?",
        (course_id,),
    ).fetchone()
    conn.close()

    if not row:
        return jsonify({"success": False, "message": "课程不存在"}), 404

    file_path = row["file_path"]
    if not file_path or not os.path.exists(file_path):
        # 如果路径不存在，从 uploads/courses 目录查找
        alt_path = os.path.join(UPLOAD_DIR, file_path)
        if os.path.exists(alt_path):
            file_path = alt_path
        else:
            return jsonify({"success": False, "message": "文件不存在"}), 404

    filename = os.path.basename(file_path)
    return send_file(file_path, as_attachment=True, download_name=filename)
