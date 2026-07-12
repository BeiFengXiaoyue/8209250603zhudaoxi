import os
import time
from flask import Blueprint, request, jsonify, send_file, Response

course_upload_bp = Blueprint("course_upload", __name__)

UPLOAD_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "uploads", "courses")


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

    upload_time = time.strftime("%Y/%m/%d %H:%M", time.localtime())

    conn = get_db()
    cursor = conn.execute(
        """INSERT INTO courses (course, teacher, time, file_path, class, description, subject, function)
           VALUES (?, ?, ?, ?, ?, ?, ?, ?)""",
        (name, teacher, upload_time, safe_filename, class_val, description, subject, function),
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
    """视频流播放（支持 HTTP Range 请求，用于 QMediaPlayer 进度拖动）"""
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

    file_size = os.path.getsize(filepath)

    # MIME 类型映射
    ext = os.path.splitext(row["file_path"])[1].lower()
    mime_map = {
        ".mp4": "video/mp4",
        ".mkv": "video/x-matroska",
        ".webm": "video/webm",
        ".avi": "video/x-msvideo",
        ".mov": "video/quicktime",
        ".wmv": "video/x-ms-wmv",
        ".flv": "video/x-flv",
        ".ts": "video/mp2t",
        ".m4v": "video/mp4",
        ".3gp": "video/3gpp",
    }
    content_type = mime_map.get(ext, "application/octet-stream")

    range_header = request.headers.get("Range", "")
    if range_header and range_header.startswith("bytes="):
        # 解析 Range: bytes=start-end
        try:
            range_str = range_header[6:]  # 去掉 "bytes="
            if "-" in range_str:
                parts = range_str.split("-", 1)
                start = int(parts[0]) if parts[0] else 0
                end = int(parts[1]) if parts[1] else file_size - 1
            else:
                start = int(range_str)
                end = file_size - 1

            if start >= file_size:
                return jsonify({"success": False, "message": "范围超出文件大小"}), 416

            end = min(end, file_size - 1)
            length = end - start + 1

            with open(filepath, "rb") as f:
                f.seek(start)
                data = f.read(length)

            response = Response(
                data,
                206,
                headers={
                    "Content-Range": f"bytes {start}-{end}/{file_size}",
                    "Content-Type": content_type,
                    "Content-Length": str(length),
                    "Accept-Ranges": "bytes",
                },
            )
            return response
        except (ValueError, OSError):
            pass

    # 无 Range 头 → 返回完整文件
    download_name = row["course"]
    return send_file(filepath, as_attachment=False, download_name=download_name, mimetype=content_type)
