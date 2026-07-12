import sqlite3
import os

DB_DIR = os.path.dirname(os.path.abspath(__file__))
DB_PATH = os.path.join(DB_DIR, "now.db")
def get_db_path():
    return DB_PATH

def user_exists(username):
    """检查用户是否存在于数据库"""
    conn = sqlite3.connect(DB_PATH)
    row = conn.execute("SELECT user_id FROM users WHERE username = ?", (username,)).fetchone()
    conn.close()
    return row is not None
def init_db():
    """初始化数据库，创建所有表（A~E）"""
    conn = sqlite3.connect(DB_PATH)

    # 数据库（A）——用户表
    conn.execute(
        """CREATE TABLE IF NOT EXISTS users (
            user_id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            keyword TEXT NOT NULL,
            class INTEGER NOT NULL,
            role TEXT NOT NULL DEFAULT 'student',
            fail_count INTEGER DEFAULT 0,
            lock_until INTEGER,
            avatar_path TEXT
        )"""
    )
    # 数据库（B）——课程视频表
    conn.execute(
        """CREATE TABLE IF NOT EXISTS courses (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            course TEXT NOT NULL,
            teacher TEXT NOT NULL,
            time TEXT NOT NULL,
            file_path TEXT NOT NULL,
            class INTEGER NOT NULL,
            duration INTEGER NOT NULL DEFAULT 0,
            description TEXT DEFAULT '',
            subject TEXT NOT NULL DEFAULT '',
            function TEXT NOT NULL DEFAULT ''
        )"""
    )

    # 兼容迁移：为已有数据库补充新列（逐条 ADD，忽略已存在列的错误）
    for col in [
        "duration INTEGER NOT NULL DEFAULT 0",
        "description TEXT DEFAULT ''",
        "subject TEXT NOT NULL DEFAULT ''",
        "function TEXT NOT NULL DEFAULT ''",
        "thumbnail_path TEXT DEFAULT ''",
    ]:
        try:
            conn.execute(f"ALTER TABLE courses ADD COLUMN {col}")
        except Exception:
            pass

    # 数据库（C）——资源表
    conn.execute(
        """CREATE TABLE IF NOT EXISTS resources (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            uploader TEXT NOT NULL,
            file_path TEXT NOT NULL,
            class INTEGER NOT NULL,
            time TEXT NOT NULL,
            course TEXT NOT NULL
        )"""
    )
    # 数据库（D）——问答表
    conn.execute(
        """CREATE TABLE IF NOT EXISTS questions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            uploader TEXT NOT NULL,
            post_id INTEGER NOT NULL,
            parent_id INTEGER NOT NULL DEFAULT 0,
            class INTEGER NOT NULL,
            time TEXT NOT NULL,
            course TEXT NOT NULL,
            content TEXT NOT NULL,
            like_count INTEGER DEFAULT 0
        )"""
    )
    # 数据库（E）——弹幕表
    conn.execute(
        """CREATE TABLE IF NOT EXISTS danmaku (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            video_id INTEGER NOT NULL,
            sender TEXT NOT NULL,
            content TEXT NOT NULL CHECK(length(content) <= 50),
            play_time INTEGER NOT NULL,
            class INTEGER NOT NULL,
            create_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )"""
    )
    # 弹幕表索引
    conn.execute("CREATE INDEX IF NOT EXISTS idx_danmaku_video ON danmaku(video_id, id)")
    conn.execute("CREATE INDEX IF NOT EXISTS idx_danmaku_class ON danmaku(class)")
    conn.execute("CREATE INDEX IF NOT EXISTS idx_danmaku_create ON danmaku(create_time)")
    # 数据库（F）——浏览历史表
    conn.execute(
        """CREATE TABLE IF NOT EXISTS view_history (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL,
            video_id INTEGER NOT NULL,
            video_title TEXT,
            teacher TEXT,
            class INTEGER NOT NULL,
            view_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )"""
    )
    conn.execute("CREATE INDEX IF NOT EXISTS idx_history_user ON view_history(username, view_time)")
    conn.execute("CREATE INDEX IF NOT EXISTS idx_history_class ON view_history(class)")

    # 数据库（G）——下载记录表
    conn.execute(
        """CREATE TABLE IF NOT EXISTS download_history (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL,
            file_name TEXT NOT NULL,
            file_type TEXT,
            file_size INTEGER,
            class INTEGER NOT NULL,
            download_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )"""
    )
    conn.execute("CREATE INDEX IF NOT EXISTS idx_download_user ON download_history(username, download_time)")

    # 数据库（H）——收藏夹表
    conn.execute(
        """CREATE TABLE IF NOT EXISTS favorites (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL,
            item_type TEXT NOT NULL CHECK(item_type IN ('video', 'resource')),
            item_id INTEGER NOT NULL,
            item_title TEXT,
            class INTEGER NOT NULL,
            added_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )"""
    )
    conn.execute("CREATE INDEX IF NOT EXISTS idx_fav_user ON favorites(username, class)")
    conn.execute("CREATE INDEX IF NOT EXISTS idx_fav_item ON favorites(item_type, item_id)")

    # 点赞记录表
    conn.execute(
        """CREATE TABLE IF NOT EXISTS question_likes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            post_id INTEGER NOT NULL,
            username TEXT NOT NULL,
            UNIQUE(post_id, username)
        )"""
    )

    conn.commit()
    conn.close()
    migrate()

def migrate():
    """数据库迁移：向 resources 表补充缺失字段"""
    conn = sqlite3.connect(DB_PATH)
    existing = [row[1] for row in conn.execute("PRAGMA table_info(resources)").fetchall()]
    migrations = [
        ("name", "ALTER TABLE resources ADD COLUMN name TEXT"),
        ("file_format", "ALTER TABLE resources ADD COLUMN file_format TEXT"),
        ("stage", "ALTER TABLE resources ADD COLUMN stage TEXT DEFAULT ''"),
        ("file_size", "ALTER TABLE resources ADD COLUMN file_size INTEGER DEFAULT 0"),
    ]
    for col, sql in migrations:
        if col not in existing:
            conn.execute(sql)
    conn.commit()
    conn.close()

if __name__ == "__main__":
    init_db()
    print(f"数据库已初始化: {DB_PATH}")