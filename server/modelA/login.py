from flask import Blueprint, request, jsonify
import hashlib
import time

login_bp = Blueprint("login", __name__)

LOCK_DURATION = 30 * 60  # 锁定 30 分钟（秒）
MAX_FAIL_COUNT = 5       # 最大失败次数


def get_db():
    """获取数据库连接（自动初始化）"""
    from database.init import get_db_path, init_db
    import sqlite3
    db_path = get_db_path()
    init_db()
    conn = sqlite3.connect(db_path)
    conn.row_factory = sqlite3.Row
    return conn


def verify_password(stored: str, password: str) -> bool:
    """验证密码（直接 SHA-256 比对）"""
    import hashlib
    return stored == hashlib.sha256(password.encode()).hexdigest()


@login_bp.route("/login", methods=["POST"])
def login():
    """
    用户登录
    请求体: {username, password}
    返回: {success, message, role?, class?}
    """
    data = request.get_json(silent=True)
    if not data:
        return jsonify({"success": False, "message": "请求体为空"}), 400

    username = data.get("username", "").strip()
    password = data.get("password", "")

    if not username or not password:
        return jsonify({"success": False, "message": "用户名和密码不能为空"}), 400

    conn = get_db()
    user = conn.execute(
        "SELECT user_id, username, keyword, class, role, fail_count, lock_until, avatar_path "
        "FROM users WHERE username = ?",
        (username,),
    ).fetchone()

    # 用户名不存在
    if not user:
        conn.close()
        return jsonify({"success": False, "message": "用户名不存在"}), 401

    now = int(time.time())
    lock_until = user["lock_until"]

    # 账号已锁定
    if lock_until and now < lock_until:
        remaining = lock_until - now
        conn.close()
        return jsonify({
            "success": False,
            "message": f"账号已锁定，请 {remaining // 60} 分钟后再试",
            "remaining_seconds": remaining,
        }), 403

    # 验证密码
    if not verify_password(user["keyword"], password):
        fail_count = (user["fail_count"] or 0) + 1

        if fail_count >= MAX_FAIL_COUNT:
            # 达到最大失败次数，锁定账号
            conn.execute(
                "UPDATE users SET fail_count = ?, lock_until = ? WHERE username = ?",
                (fail_count, now + LOCK_DURATION, username),
            )
            conn.commit()
            conn.close()
            return jsonify({
                "success": False,
                "message": "密码错误次数过多，账号已锁定 30 分钟",
            }), 403
        else:
            conn.execute(
                "UPDATE users SET fail_count = ? WHERE username = ?",
                (fail_count, username),
            )
            conn.commit()
            conn.close()
            return jsonify({
                "success": False,
                "message": "密码错误",
                "remaining_attempts": MAX_FAIL_COUNT - fail_count,
            }), 401

    # 登录成功，重置失败计数
    conn.execute(
        "UPDATE users SET fail_count = 0, lock_until = NULL WHERE username = ?",
        (username,),
    )
    conn.commit()
    conn.close()

    return jsonify({
        "success": True,
        "message": "登录成功",
        "role": user["role"],
        "class": user["class"],
        "user_id": user["user_id"],
        "username": user["username"],
        "avatar_path": user["avatar_path"],
    }), 200
