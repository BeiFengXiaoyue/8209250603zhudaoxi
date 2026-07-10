import os
import time
from flask import Blueprint, request, jsonify, send_file

resources_bp = Blueprint("resources", __name__)

UPLOAD_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "uploads", "resources")


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


@resources_bp.route("/resources/upload", methods=["POST"])
def upload_resource():
    """
    上传资料文件
    multipart/form-data: file (文件) + username + name + course + stage + class
    """
    ensure_upload_dir()

    file = request.files.get("file")
    username = request.form.get("username", "").strip()
    name = request.form.get("name", "").strip()
    course = request.form.get("course", "").strip()
    stage = request.form.get("stage", "").strip()
    class_val = request.form.get("class", type=int)

    if not file or not username or not class_val:
        return jsonify({"success": False, "message": "缺少必填字段"}), 400

    if not name:
        name = file.filename

    # 保存文件: {timestamp}_{原始文件名}
    ts = str(int(time.time() * 1000))
    safe_filename = f"{ts}_{file.filename}"
    filepath = os.path.join(UPLOAD_DIR, safe_filename)
    file.save(filepath)

    file_size = os.path.getsize(filepath)
    file_ext = os.path.splitext(file.filename)[1].lstrip(".").upper()
    upload_time = time.strftime("%Y/%m/%d %H:%M", time.localtime())

    conn = get_db()
    cursor = conn.execute(
        """INSERT INTO resources (uploader, file_path, class, time, course, name, file_format, stage, file_size)
           VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)""",
        (username, safe_filename, class_val, upload_time, course, name, file_ext, stage, file_size),
    )
    res_id = cursor.lastrowid
    conn.commit()
    conn.close()

    return jsonify({
        "success": True,
        "data": {
            "id": res_id,
            "uploader": username,
            "name": name,
            "file_path": safe_filename,
            "class": class_val,
            "time": upload_time,
            "course": course,
            "file_format": file_ext,
            "stage": stage,
            "file_size": file_size,
        }
    }), 201


@resources_bp.route("/resources", methods=["GET"])
def get_resources():
    """
    获取/搜索资料列表
    URL 参数: class (必填) & course & stage & keyword
    空字段不参与筛选（通配）
    """
    class_val = request.args.get("class", type=int)
    course = request.args.get("course", "").strip()
    stage = request.args.get("stage", "").strip()
    keyword = request.args.get("keyword", "").strip()

    if not class_val:
        return jsonify({"success": False, "message": "缺少班级参数"}), 400

    conn = get_db()
    query = """SELECT id, uploader, file_path, class, time, course,
                      name, file_format, stage, file_size
               FROM resources WHERE class = ?"""
    params = [class_val]

    if course:
        query += " AND course = ?"
        params.append(course)
    if stage:
        query += " AND stage = ?"
        params.append(stage)
    if keyword:
        query += " AND (name LIKE ? OR file_format LIKE ?)"
        params.append(f"%{keyword}%")
        params.append(f"%{keyword}%")

    query += " ORDER BY time DESC"

    rows = conn.execute(query, params).fetchall()
    conn.close()

    data = [
        {
            "id": row["id"],
            "uploader": row["uploader"],
            "name": row["name"] or "",
            "file_path": row["file_path"],
            "class": row["class"],
            "time": row["time"],
            "course": row["course"],
            "file_format": row["file_format"] or "",
            "stage": row["stage"] or "",
            "file_size": row["file_size"] or 0,
        }
        for row in rows
    ]

    return jsonify({"success": True, "data": data}), 200


@resources_bp.route("/files/<int:res_id>", methods=["GET"])
def download_resource(res_id):
    """下载/查看资料文件"""
    conn = get_db()
    row = conn.execute("SELECT file_path, name FROM resources WHERE id = ?", (res_id,)).fetchone()
    conn.close()

    if not row:
        return jsonify({"success": False, "message": "资源不存在"}), 404

    filepath = os.path.join(UPLOAD_DIR, row["file_path"])
    if not os.path.exists(filepath):
        return jsonify({"success": False, "message": "文件不存在"}), 404

    download_name = row["name"] or row["file_path"]
    return send_file(filepath, as_attachment=False, download_name=download_name)
