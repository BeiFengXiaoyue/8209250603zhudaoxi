import os
import time
from flask import Blueprint, request, jsonify, send_file
from .ffm_tools import extract_first_frame

course_upload_bp = Blueprint("course_upload", __name__)

UPLOAD_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "uploads", "courses")
THUMB_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "uploads", "thumbs")


def get_db():
    from database.init import get_db_path, init_db
    import sqlite3
    db_path = get_db_path()
    init_db()
    conn = sqlite3.connect(db_path)
    conn.row_factory = sqlite3.Row
    return conn


def ensure_upload_dir():
    os.makedirs(UPLOAD_DIR, exist_ok=True)


@course_upload_bp.route("/courses/upload", methods=["POST"])
def upload_course():
    """
    上传课程视频
    multipart/form-data: file (视频文件) + name + subject + function + description + teacher + class
    """
    ensure_upload_dir()

    file = request.files.get("file")
    name = request.form.get("name", "").strip()
    teacher = request.form.get("teacher", "").strip()
    subject = request.form.get("subject", "").strip()
    function = request.form.get("function", "").strip()
    description = request.form.get("description", "").strip()
    class_val = request.form.get("class", type=int)

    if not file or not name or not teacher or not class_val:
        return jsonify({"success": False, "message": "缺少必填字段（视频文件、名称、教师、班级）"}), 400

    # 保存文件: {timestamp}_{原始文件名}
    ts = str(int(time.time() * 1000))
    safe_filename = f"{ts}_{file.filename}"
    filepath = os.path.join(UPLOAD_DIR, safe_filename)
    file.save(filepath)

    # 生成缩略图（首帧 JPEG）
    thumbnail_path = ""
    try:
        os.makedirs(THUMB_DIR, exist_ok=True)
        thumb_file = extract_first_frame(str(filepath), output_dir=THUMB_DIR, quality=5)
        thumbnail_path = os.path.basename(thumb_file)
    except Exception:
        pass  # 缩略图生成失败不影响上传

    upload_time = time.strftime("%Y/%m/%d %H:%M", time.localtime())

    conn = get_db()
    cursor = conn.execute(
        """INSERT INTO courses (course, teacher, time, file_path, class, description, subject, function, thumbnail_path)
           VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)""",
        (name, teacher, upload_time, safe_filename, class_val, description, subject, function, thumbnail_path),
    )
    course_id = cursor.lastrowid
    conn.commit()
    conn.close()

    return jsonify({
        "success": True,
        "data": {
            "id": course_id,
            "course": name,
            "teacher": teacher,
            "time": upload_time,
            "file_path": safe_filename,
            "class": class_val,
            "description": description,
            "subject": subject,
            "function": function,
            "thumbnail_path": thumbnail_path,
        }
    }), 201


@course_upload_bp.route("/courses/list", methods=["GET"])
def list_courses():
    """
    获取课程列表（供教师查看已上传课程）
    URL 参数: class (必填)
    """
    class_val = request.args.get("class", type=int)
    if not class_val:
        return jsonify({"success": False, "message": "缺少班级参数"}), 400

    conn = get_db()
    rows = conn.execute(
        """SELECT id, course, teacher, time, file_path, class, description, subject, function
           FROM courses WHERE class = ?
           ORDER BY time DESC""",
        (class_val,),
    ).fetchall()
    conn.close()

    data = [
        {
            "id": row["id"],
            "course": row["course"],
            "teacher": row["teacher"],
            "time": row["time"],
            "file_path": row["file_path"],
            "class": row["class"],
            "description": row["description"] or "",
            "subject": row["subject"] or "",
            "function": row["function"] or "",
        }
        for row in rows
    ]

    return jsonify({"success": True, "data": data}), 200


@course_upload_bp.route("/courses/<int:course_id>/file", methods=["GET"])
def download_course_file(course_id):
    """获取课程视频文件"""
    conn = get_db()
    row = conn.execute(
        "SELECT course, file_path FROM courses WHERE id = ?", (course_id,)
    ).fetchone()
    conn.close()

    if not row:
        return jsonify({"success": False, "message": "课程不存在"}), 404

    filepath = os.path.join(UPLOAD_DIR, row["file_path"])
    if not os.path.exists(filepath):
        return jsonify({"success": False, "message": "文件不存在"}), 404

    download_name = row["course"]
    return send_file(filepath, as_attachment=False, download_name=download_name)


@course_upload_bp.route("/courses/<int:course_id>/thumbnail", methods=["GET"])
def get_course_thumbnail(course_id):
    """获取课程视频缩略图"""
    conn = get_db()
    row = conn.execute(
        "SELECT thumbnail_path FROM courses WHERE id = ?", (course_id,)
    ).fetchone()
    conn.close()

    if not row or not row["thumbnail_path"]:
        return jsonify({"success": False, "message": "缩略图不存在"}), 404

    thumb_path = os.path.join(THUMB_DIR, row["thumbnail_path"])
    if not os.path.exists(thumb_path):
        return jsonify({"success": False, "message": "缩略图文件不存在"}), 404

    return send_file(thumb_path, mimetype="image/jpeg")
