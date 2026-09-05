#!/usr/bin/env python3
"""Desktop proof-of-concept sender for CardputerMirror.

Connect the computer to the CardputerMirror Wi-Fi network, then run:
  python pc_sender.py

Dependencies:
  pip install pillow mss
"""

import argparse
import io
import socket
import struct
import time
from PIL import Image
import mss

W, H = 240, 135
MAGIC = b"CMIR"


def cover_crop(img: Image.Image) -> Image.Image:
    img = img.convert("RGB")
    src_aspect = img.width / img.height
    dst_aspect = W / H
    if src_aspect > dst_aspect:
        crop_w = round(img.height * dst_aspect)
        left = (img.width - crop_w) // 2
        img = img.crop((left, 0, left + crop_w, img.height))
    else:
        crop_h = round(img.width / dst_aspect)
        top = (img.height - crop_h) // 2
        img = img.crop((0, top, img.width, top + crop_h))
    return img.resize((W, H), Image.Resampling.LANCZOS)


def encode_jpeg(img: Image.Image, quality: int) -> bytes:
    out = io.BytesIO()
    img.save(out, format="JPEG", quality=quality, optimize=False, progressive=False)
    return out.getvalue()


def send_frame(sock: socket.socket, jpeg: bytes) -> None:
    sock.sendall(MAGIC + struct.pack(">I", len(jpeg)) + jpeg)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--host", default="192.168.4.1")
    ap.add_argument("--port", type=int, default=9000)
    ap.add_argument("--fps", type=float, default=8.0)
    ap.add_argument("--quality", type=int, default=45)
    args = ap.parse_args()

    interval = 1.0 / max(1.0, args.fps)
    with socket.create_connection((args.host, args.port), timeout=5) as sock, mss.mss() as sct:
        sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        monitor = sct.monitors[1]
        print(f"Streaming desktop to {args.host}:{args.port} at {args.fps:g} fps. Ctrl+C to stop.")
        next_at = time.monotonic()
        while True:
            shot = sct.grab(monitor)
            img = Image.frombytes("RGB", shot.size, shot.rgb)
            frame = cover_crop(img)
            send_frame(sock, encode_jpeg(frame, args.quality))
            next_at += interval
            time.sleep(max(0.0, next_at - time.monotonic()))


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        pass
