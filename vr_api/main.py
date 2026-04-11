from fastapi import FastAPI
from pydantic import BaseModel
import paho.mqtt.client as mqtt
import json
import os

app = FastAPI(title="Lamp API Bridge")

# === Настройки MQTT ===
MQTT_HOST = os.getenv("MQTT_HOST", "127.0.0.1")
MQTT_PORT = int(os.getenv("MQTT_PORT", "1883"))
MQTT_USER = os.getenv("MQTT_USER", "")
MQTT_PASS = os.getenv("MQTT_PASS", "")

# friendly_name из Zigbee2MQTT
DEVICE = "moes_bulb"
# Топики Zigbee2MQTT
TOPIC_SET = f"zigbee2mqtt/{DEVICE}/set"
TOPIC_GET = f"zigbee2mqtt/{DEVICE}/get"
TOPIC_STATE = f"zigbee2mqtt/{DEVICE}"

last_state = {}

client = mqtt.Client()

if MQTT_USER:
    client.username_pw_set(MQTT_USER, MQTT_PASS)

def on_message(_client, _userdata, msg):
    global last_state
    try:
        payload = msg.payload.decode("utf-8")
        last_state = json.loads(payload)
    except:
        pass

client.on_message = on_message
client.connect(MQTT_HOST, MQTT_PORT, 60)
client.subscribe(TOPIC_STATE)
client.loop_start()

class BrightnessBody(BaseModel):
    value: int  # 0..254 (Zigbee2MQTT), мы можем принимать 0..100 и конвертировать — позже

@app.post("/lamp/on")
def lamp_on():
    client.publish(TOPIC_SET, json.dumps({"state": "ON"}))
    return {"ok": True}

@app.post("/lamp/off")
def lamp_off():
    client.publish(TOPIC_SET, json.dumps({"state": "OFF"}))
    return {"ok": True}

@app.post("/lamp/brightness")
def lamp_brightness(body: BrightnessBody):
    # ожидаем 0..254 (Z2M). Если хочешь 0..100 — скажи, сделаю конвертацию.
    v = max(0, min(254, body.value))
    client.publish(TOPIC_SET, json.dumps({"brightness": v}))
    return {"ok": True, "brightness": v}

class ColorRGBBody(BaseModel):
    r: int  # 0..255
    g: int  # 0..255
    b: int  # 0..255

@app.post("/lamp/color_rgb")
def lamp_color_rgb(body: ColorRGBBody):
    # sRGB → linear → XYZ → xy
    def to_linear(c):
        c = c / 255.0
        return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4

    rl, gl, bl = to_linear(body.r), to_linear(body.g), to_linear(body.b)
    X = rl * 0.4124 + gl * 0.3576 + bl * 0.1805
    Y = rl * 0.2126 + gl * 0.7152 + bl * 0.0722
    Z = rl * 0.0193 + gl * 0.1192 + bl * 0.9505
    s = X + Y + Z
    if s == 0:
        x, y = 0.3127, 0.3290  # D65 white
    else:
        x, y = X / s, Y / s

    client.publish(TOPIC_SET, json.dumps({"color": {"x": round(x, 4), "y": round(y, 4)}}))
    return {"ok": True, "color": {"x": x, "y": y}}

@app.get("/lamp/state")
def lamp_state():
    # можно ещё отправлять TOPIC_GET, но для MVP достаточно кэша last_state
    return {"ok": True, "state": last_state}