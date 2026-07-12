"""
ffm_tools — 基于 FFmpeg 的视频处理工具函数

当前提供:
    extract_first_frame(video_path, output_dir, quality)
        提取视频第一帧并保存为 JPEG，返回输出路径。
"""

import subprocess
import shutil
from pathlib import Path
from typing import Optional


def extract_first_frame(
    video_path: str,
    output_dir: Optional[str] = None,
    quality: int = 2,
) -> str:
    """
    提取视频的第一帧并保存为 JPEG 图片。

    参数
    ----------
    video_path : str
        输入视频文件的路径。
    output_dir : Optional[str]
        输出目录。为 None 时默认保存到 ./picture。
    quality : int
        JPEG 画质，范围 2-31（数字越小质量越高）。默认 2。

    返回
    -------
    str
        生成的 JPEG 图片的完整路径。

    异常
    -----
    FileNotFoundError
        输入视频文件不存在。
    RuntimeError
        FFmpeg 未找到或执行失败。
    """
    video = Path(video_path)

    # ---------- 检查输入文件 ----------
    if not video.is_file():
        raise FileNotFoundError(f"视频文件不存在: {video_path}")

    # ---------- 检查 FFmpeg 是否可用 ----------
    if shutil.which("ffmpeg") is None:
        raise RuntimeError(
            "未找到 ffmpeg，请确保它已安装并已添加到系统环境变量 PATH 中。"
        )

    # ---------- 确定输出路径 ----------
    out_dir = Path(output_dir) if output_dir else Path("./picture")
    out_dir.mkdir(parents=True, exist_ok=True)
    output_path = out_dir / f"{video.stem}.jpg"

    # ---------- 构造 FFmpeg 命令 ----------
    cmd = [
        "ffmpeg",
        "-ss", "0",              # 快速跳转到开头
        "-i", str(video),        # 输入文件
        "-vframes", "1",         # 只取 1 帧
        "-q:v", str(quality),    # JPEG 质量
        "-y",                    # 覆盖已有文件
        str(output_path),
    ]

    # ---------- 执行 ----------
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=60,
        )
    except subprocess.TimeoutExpired:
        raise RuntimeError(f"FFmpeg 处理超时（60 秒）: {video_path}")
    except FileNotFoundError:
        raise RuntimeError(
            "未找到 ffmpeg，请确保它已安装并已添加到系统环境变量 PATH 中。"
        )

    if result.returncode != 0:
        raise RuntimeError(
            f"FFmpeg 执行失败（返回码 {result.returncode}）:\n"
            f"{result.stderr.strip()}"
        )

    # ---------- 验证输出 ----------
    if not output_path.is_file():
        raise RuntimeError(
            f"FFmpeg 执行完毕但未生成输出文件: {output_path}"
        )

    return str(output_path)
