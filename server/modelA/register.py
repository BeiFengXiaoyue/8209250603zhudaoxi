from flask import Blueprint, request, jsonify

auth_bp = Blueprint("auth", __name__)


def get_db():
    """获取数据库连接（自动初始化）"""
    from database.init import get_db_path, init_db
    import sqlite3
    db_path = get_db_path()
    init_db()
    conn = sqlite3.connect(db_path)
    conn.row_factory = sqlite3.Row
    return conn


def validate_username(username):
    """校验用户名：3-20位字母、数字、下划线"""
    import re
    if not username:
        return False, "用户名不能为空"
    if not re.match(r'^\w{3,20}$', username):
        return False, "用户名需为3-20位字母、数字或下划线"
    return True, ""


def validate_password(password):
    """服务端不做密码校验（客户端已完成）"""
    if not password:
        return False, "密码不能为空"
    return True, ""


def hash_password(password):
    """哈希密码（SHA-256，无 salt）"""
    import hashlib
    return hashlib.sha256(password.encode()).hexdigest()


@auth_bp.route("/register", methods=["POST"])
def register():
    """
    注册新用户
    请求体: {username, password, class, role}
    返回: {success, message, user_id?}
    """
    data = request.get_json(silent=True)
    if not data:
        return jsonify({"success": False, "message": "请求体为空"}), 400

    username = data.get("username", "").strip()
    password = data.get("password", "")
    class_val = data.get("class", 0)
    role = data.get("role", "student")

    # 校验角色
    if role not in ("teacher", "student"):
        return jsonify({"success": False, "message": "角色只能是 teacher 或 student"}), 400

    # 校验班级
    try:
        class_val = int(class_val)
    except (TypeError, ValueError):
        return jsonify({"success": False, "message": "班级号必须为整数"}), 400

    # 校验用户名
    valid, msg = validate_username(username)
    if not valid:
        return jsonify({"success": False, "message": msg}), 400

    # 校验密码
    valid, msg = validate_password(password)
    if not valid:
        return jsonify({"success": False, "message": msg}), 400

    # 检查用户名是否已存在
    conn = get_db()
    existing = conn.execute(
        "SELECT user_id FROM users WHERE username = ?", (username,)
    ).fetchone()

    if existing:
        conn.close()
        return jsonify({"success": False, "message": "用户名已存在"}), 409

    # 插入新用户
    hashed = hash_password(password)
    cursor = conn.execute(
        """INSERT INTO users (username, keyword, class, role)
           VALUES (?, ?, ?, ?)""",
        (username, hashed, class_val, role),
    )
    user_id = cursor.lastrowid
    conn.commit()
    conn.close()

    return jsonify({
        "success": True,
        "message": "注册成功",
        "user_id": user_id,
    }), 201
