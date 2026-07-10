from flask import Blueprint, request, jsonify

student_bp = Blueprint("student_manage", __name__)


def get_db():
    from database.init import get_db_path, init_db
    import sqlite3
    db_path = get_db_path()
    init_db()
    conn = sqlite3.connect(db_path)
    conn.row_factory = sqlite3.Row
    return conn


@student_bp.route("/students", methods=["GET"])
def get_students():
    """获取某班级的学生列表"""
    class_id = request.args.get("class_id", type=int)

    if not class_id:
        return jsonify({"success": False, "message": "缺少 class_id 参数"}), 400

    conn = get_db()
    rows = conn.execute(
        """SELECT user_id, username, role, avatar_path
           FROM users
           WHERE class = ? AND role = 'student'
           ORDER BY username ASC""",
        (class_id,),
    ).fetchall()
    conn.close()

    return jsonify({
        "success": True,
        "data": [dict(row) for row in rows],
    }), 200


@student_bp.route("/students/<username>", methods=["DELETE"])
def delete_student(username):
    """删除学生账号"""
    conn = get_db()

    # 验证用户存在且为学生
    user = conn.execute(
        "SELECT user_id, role FROM users WHERE username = ?",
        (username,),
    ).fetchone()

    if not user:
        conn.close()
        return jsonify({"success": False, "message": "用户不存在"}), 404

    if user["role"] != "student":
        conn.close()
        return jsonify({"success": False, "message": "只能删除学生账号"}), 400

    # 删除相关数据（级联清理）
    conn.execute("DELETE FROM questions WHERE uploader = ?", (username,))
    conn.execute("DELETE FROM view_history WHERE username = ?", (username,))
    conn.execute("DELETE FROM download_history WHERE username = ?", (username,))
    conn.execute("DELETE FROM favorites WHERE username = ?", (username,))
    conn.execute("DELETE FROM question_likes WHERE username = ?", (username,))
    conn.execute("DELETE FROM users WHERE username = ?", (username,))

    conn.commit()
    conn.close()

    return jsonify({"success": True, "message": f"已删除 {username}"}), 200


@student_bp.route("/students/<username>/reset-password", methods=["POST"])
def reset_password(username):
    """重置学生密码为默认密码 123456（客户端 SHA-256 后存储）"""
    import hashlib

    conn = get_db()
    user = conn.execute(
        "SELECT user_id, role FROM users WHERE username = ?",
        (username,),
    ).fetchone()

    if not user:
        conn.close()
        return jsonify({"success": False, "message": "用户不存在"}), 404

    if user["role"] != "student":
        conn.close()
        return jsonify({"success": False, "message": "只能重置学生密码"}), 400

    # 默认密码 "123456" 的二次 SHA-256（客户端一次 + 服务端一次，与注册流程一致）
    once = hashlib.sha256("123456".encode()).hexdigest()
    default_hash = hashlib.sha256(once.encode()).hexdigest()
    conn.execute(
        "UPDATE users SET keyword = ? WHERE username = ?",
        (default_hash, username),
    )
    conn.commit()
    conn.close()

    return jsonify({"success": True, "message": f"{username} 的密码已重置为 123456"}), 200
