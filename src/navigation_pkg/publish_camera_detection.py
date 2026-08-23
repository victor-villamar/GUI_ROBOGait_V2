#!/usr/bin/env python3

from pathlib import Path
import argparse
import sys
import time

from PIL import Image
import rclpy
from navigation_pkg.msg import CameraDetection


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_LOGOS_DIR = REPOSITORY_ROOT / "src/robogait_gui/resources/Logos"
DEFAULT_TOPIC = "/camera_detection"
IMAGE_EXTENSIONS = {".png", ".jpg", ".jpeg", ".bmp", ".webp"}


def list_images(logos_dir):
    if not logos_dir.exists():
        raise FileNotFoundError(f"No existe el directorio: {logos_dir}")

    images = sorted(path for path in logos_dir.iterdir() if path.is_file() and path.suffix.lower() in IMAGE_EXTENSIONS)
    if not images:
        raise FileNotFoundError(f"No hay imagenes en: {logos_dir}")

    return images


def select_image(images, requested_image=None):
    if requested_image:
        requested_path = Path(requested_image)
        if requested_path.is_absolute():
            if requested_path.exists():
                return requested_path
            raise FileNotFoundError(f"No existe la imagen: {requested_path}")

        matches = [path for path in images if path.name == requested_image]
        if len(matches) == 1:
            return matches[0]

        partial_matches = [path for path in images if requested_image.lower() in path.name.lower()]
        if len(partial_matches) == 1:
            return partial_matches[0]

        if len(partial_matches) > 1:
            names = ", ".join(path.name for path in partial_matches)
            raise ValueError(f"Hay varias coincidencias para '{requested_image}': {names}")

        raise FileNotFoundError(f"No se encontro '{requested_image}' en Logos")

    print("\nImagenes disponibles:")
    for index, path in enumerate(images, start=1):
        print(f"  {index}. {path.name}")

    while True:
        choice = input("Elige una imagen por numero [Ctrl+C para salir]: ").strip()
        if not choice:
            continue

        if choice.isdigit():
            index = int(choice)
            if 1 <= index <= len(images):
                return images[index - 1]
            print(f"Numero fuera de rango: {index}")
            continue

        print("Entrada no valida. Introduce el numero de una imagen.")


def load_image(image_path, max_size):
    image = Image.open(image_path).convert("RGBA")
    if max_size > 0:
        image.thumbnail((max_size, max_size))
    return image


def build_message(node, image, detection):
    width, height = image.size

    msg = CameraDetection()
    msg.user_detection.detection = detection
    msg.user_detection.x = 0.0
    msg.user_detection.y = 0.0
    msg.user_detection.z = 0.0

    msg.snapshot.header.frame_id = "camera"
    msg.snapshot.height = height
    msg.snapshot.width = width
    msg.snapshot.encoding = "rgba8"
    msg.snapshot.is_bigendian = 0
    msg.snapshot.step = width * 4
    msg.snapshot.data = list(image.tobytes())
    msg.snapshot.header.stamp = node.get_clock().now().to_msg()

    return msg


def publish_image(node, pub, topic, image_path, detection, count, max_size):
    image = load_image(image_path, max_size)
    msg = build_message(node, image, detection)

    print(f"\nPublicando {image_path.name} en {topic}")
    print(f"Imagen: {msg.snapshot.width}x{msg.snapshot.height}, encoding={msg.snapshot.encoding}, detection={detection}")

    for _ in range(max(1, count)):
        msg.snapshot.header.stamp = node.get_clock().now().to_msg()
        pub.publish(msg)
        rclpy.spin_once(node, timeout_sec=0.1)
        time.sleep(0.1)

    print("Mensaje enviado.")


def parse_args():
    parser = argparse.ArgumentParser(description="Publica navigation_pkg/msg/CameraDetection con una imagen de resources/Logos.")
    parser.add_argument("-i", "--image", help="Nombre, coincidencia parcial o ruta absoluta de la imagen a enviar.")
    parser.add_argument("-t", "--topic", default=DEFAULT_TOPIC, help=f"Topic de salida. Por defecto: {DEFAULT_TOPIC}")
    parser.add_argument("-d", "--detection", type=int, default=1, help="Valor detection. Por defecto: 1")
    parser.add_argument("-n", "--count", type=int, default=10, help="Numero de publicaciones. Por defecto: 10")
    parser.add_argument("--max-size", type=int, default=800, help="Tamano maximo del lado mayor. 0 mantiene tamano original.")
    parser.add_argument("--logos-dir", type=Path, default=DEFAULT_LOGOS_DIR, help=f"Directorio de imagenes. Por defecto: {DEFAULT_LOGOS_DIR}")
    return parser.parse_args()


def main():
    args = parse_args()

    try:
        images = list_images(args.logos_dir)
    except Exception as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1

    node = None
    exit_code = 0

    try:
        rclpy.init()
        node = rclpy.create_node("camera_detection_terminal_pub")
        pub = node.create_publisher(CameraDetection, args.topic, 10)

        if args.image:
            image_path = select_image(images, args.image)
            publish_image(node, pub, args.topic, image_path, args.detection, args.count, args.max_size)
        else:
            while rclpy.ok():
                image_path = select_image(images)
                publish_image(node, pub, args.topic, image_path, args.detection, args.count, args.max_size)
    except KeyboardInterrupt:
        print("\nSaliendo...")
    except Exception as exc:
        print(f"Error: {exc}", file=sys.stderr)
        exit_code = 1
    finally:
        if node is not None:
            node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()

    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
