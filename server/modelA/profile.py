from flask import Blueprint, request, jsonify, send_file
import hashlib
import os
from database.init import user_exists

profile_bp = Blueprint("profile", __name__)

UPLOAD_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "uploads", "avatars")


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


@profile_bp.route("/change-password", methods=["POST"])
def change_password():
    data = request.get_json(silent=True)
    if not data:
        return jsonify({"success": False, "message": "\u8bf7\u6c42\u4f53\u4e3a\u7a7a"}), 400
    username = data.get("username", "").strip()
    old_password = data.get("old_password", "")
    new_password = data.get("new_password", "")
    if not username or not old_password or not new_password:
        return jsonify({"success": False, "message": "\u7f3a\u5c11\u5fc5\u8981\u53c2\u6570"}), 400
    conn = get_db()
    user = conn.execute("SELECT keyword FROM users WHERE username = ?", (username,)).fetchone()
    if not user:
        conn.close()
        return jsonify({"success": False, "message": "\u7528\u6237\u4e0d\u5b58\u5728"}), 404
    if user["keyword"] != hashlib.sha256(old_password.encode()).hexdigest():
        conn.close()
        return jsonify({"success": False, "message": "\u65e7\u5bc6\u7801\u9519\u8bef"}), 401
    new_hash = hashlib.sha256(new_password.encode()).hexdigest()
    conn.execute("UPDATE users SET keyword = ? WHERE username = ?", (new_hash, username))
    conn.commit()
    conn.close()
    return jsonify({"success": True, "message": "\u5bc6\u7801\u4fee\u6539\u6210\u529f"}), 200


@profile_bp.route("/change-username", methods=["POST"])
def change_username():
    data = request.get_json(silent=True)
    if not data:
        return jsonify({"success": False, "message": "\u8bf7\u6c42\u4f53\u4e3a\u7a7a"}), 400
    username = data.get("username", "").strip()
    new_username = data.get("new_username", "").strip()
    if not username or not new_username:
        return jsonify({"success": False, "message": "\u7f3a\u5c11\u5fc5\u8981\u53c2\u6570"}), 400
    import re
    if not re.match(r'^\w{3,20}$', new_username):
        return jsonify({"success": False, "message": "\u7528\u6237\u540d\u9700\u4e3a3-20\u4f4d\u5b57\u6bcd\u3001\u6570\u5b57\u6216\u4e0b\u5212\u7ebf"}), 400
    conn = get_db()
    user = conn.execute("SELECT user_id FROM users WHERE username = ?", (username,)).fetchone()
    if not user:
        conn.close()
        return jsonify({"success": False, "message": "\u7528\u6237\u4e0d\u5b58\u5728"}), 404
    existing = conn.execute("SELECT user_id FROM users WHERE username = ?", (new_username,)).fetchone()
    if existing:
        conn.close()
        return jsonify({"success": False, "message": "\u7528\u6237\u540d\u5df2\u5b58\u5728"}), 409
    tables = [("users","username"),("courses","teacher"),("resources","uploader"),("questions","uploader"),("danmaku","sender"),("view_history","username"),("download_history","username"),("favorites","username")]
    for table, column in tables:
        conn.execute(f"UPDATE {table} SET {column} = ? WHERE {column} = ?", (new_username, username))
    conn.commit()
    conn.close()
    return jsonify({"success": True, "message": "\u7528\u6237\u540d\u4fee\u6539\u6210\u529f", "new_username": new_username}), 200


@profile_bp.route("/upload-avatar", methods=["POST"])
def upload_avatar():
    username = request.form.get("username", "").strip()
    if not username:
        return jsonify({"success": False, "message": "\u7f3a\u5c11\u7528\u6237\u540d"}), 400
    if "file" not in request.files:
        return jsonify({"success": False, "message": "\u672a\u9009\u62e9\u6587\u4ef6"}), 400
    file = request.files["file"]
    if file.filename == "":
        return jsonify({"success": False, "message": "\u6587\u4ef6\u540d\u4e3a\u7a7a"}), 400
    conn = get_db()
    user = conn.execute("SELECT user_id FROM users WHERE username = ?", (username,)).fetchone()
    if not user:
        conn.close()
        return jsonify({"success": False, "message": "\u7528\u6237\u4e0d\u5b58\u5728"}), 404
    user_id = user["user_id"]
    ensure_upload_dir()
    import time
    ext = os.path.splitext(file.filename)[1] or ".png"
    filename = f"{user_id}_{int(time.time())}{ext}"
    file.save(os.path.join(UPLOAD_DIR, filename))
    conn.execute("UPDATE users SET avatar_path = ? WHERE username = ?", (filename, username))
    conn.commit()
    conn.close()
    return jsonify({"success": True, "message": "\u5934\u50cf\u4e0a\u4f20\u6210\u529f", "avatar_url": f"/api/user/avatar/{username}"}), 200


@profile_bp.route("/avatar/<username>", methods=["GET"])
def get_avatar(username):
    conn = get_db()
    user = conn.execute("SELECT avatar_path FROM users WHERE username = ?", (username,)).fetchone()
    conn.close()
    if not user or not user["avatar_path"]:
        return jsonify({"success": False, "message": "\u672a\u8bbe\u7f6e\u5934\u50cf"}), 404
    filepath = os.path.join(UPLOAD_DIR, user["avatar_path"])
    if not os.path.exists(filepath):
        return jsonify({"success": False, "message": "\u5934\u50cf\u6587\u4ef6\u4e0d\u5b58\u5728"}), 404
    return send_file(filepath, mimetype="image/png")
