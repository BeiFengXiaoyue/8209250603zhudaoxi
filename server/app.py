from flask import Flask

from modelA.register import auth_bp
from modelA.login import login_bp
from modelA.courses import courses_bp
from modelA.user_data import user_bp
from modelA.profile import profile_bp
from modelA.questions import questions_bp
from modelA.student_manage import student_bp
from modelA.resources import resources_bp
from modelA.danmaku import danmaku_bp
from modelA.files import files_bp
from modelA.course_upload import course_upload_bp

app = Flask(__name__)
app.register_blueprint(auth_bp, url_prefix="/api/auth")
app.register_blueprint(login_bp, url_prefix="/api/auth")
app.register_blueprint(courses_bp, url_prefix="/api")
app.register_blueprint(user_bp, url_prefix="/api/user")
app.register_blueprint(profile_bp, url_prefix="/api/user")
app.register_blueprint(questions_bp, url_prefix="/api")
app.register_blueprint(student_bp, url_prefix="/api/teacher")
app.register_blueprint(resources_bp, url_prefix="/api")
app.register_blueprint(danmaku_bp, url_prefix="/api")
app.register_blueprint(files_bp, url_prefix="/api")
app.register_blueprint(course_upload_bp, url_prefix="/api")


@app.route('/')
def index():
    return 'Hello, World!'


if __name__ == '__main__':
    app.run(host='127.0.0.1', port=5000, debug=True)
