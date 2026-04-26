from fastapi import FastAPI
from fastapi.responses import JSONResponse
from pydantic import BaseModel
from starlette.requests import ClientDisconnect

import paho.mqtt.client as mqtt
import json
import os

app = FastAPI(title="Lamp API Bridge")

MQTT_HOST = os.getenv("MQTT_HOST", "127.0.0.1")
MQTT_PORT = int(os.getenv("MQTT_PORT", "1883"))
MQTT_USER = os.getenv("MQTT_USER", "")
MQTT_PASS = os.getenv("MQTT_PASS", "")

DEVICE = "moes_bulb"
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
        data = json.loads(payload)
        old_rgb = {
            "color_r": last_state.get("color_r"),
            "color_g": last_state.get("color_g"),
            "color_b": last_state.get("color_b"),
        }
        last_state = data
        # Restore RGB if MQTT didn't provide them
        for key, val in old_rgb.items():
            if val is not None and key not in last_state:
                last_state[key] = val
    except:
        pass

client.on_message = on_message
client.connect(MQTT_HOST, MQTT_PORT, 60)
client.subscribe(TOPIC_STATE)
client.loop_start()

client.publish(TOPIC_GET, json.dumps({"state": ""}))



@app.middleware("http")
async def catch_disconnect(request, call_next):
    try:
        return await call_next(request)
    except ClientDisconnect:
        return JSONResponse({"status": "client disconnected"}, status_code=499)



class BrightnessBody(BaseModel):
    value: int

class ColorRGBBody(BaseModel):
    r: int
    g: int
    b: int

class ColorTempBody(BaseModel):
    value: int



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
    v = max(0, min(254, body.value))
    client.publish(TOPIC_SET, json.dumps({"brightness": v}))
    last_state["brightness"] = v
    return {"ok": True, "brightness": v}

@app.post("/lamp/color_rgb")
def lamp_color_rgb(body: ColorRGBBody):
    r = max(0, min(255, body.r))
    g = max(0, min(255, body.g))
    b = max(0, min(255, body.b))

    def to_linear(c):
        c = c / 255.0
        return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4

    rl, gl, bl = to_linear(r), to_linear(g), to_linear(b)
    X = rl * 0.4124 + gl * 0.3576 + bl * 0.1805
    Y = rl * 0.2126 + gl * 0.7152 + bl * 0.0722
    Z = rl * 0.0193 + gl * 0.1192 + bl * 0.9505
    s = X + Y + Z
    if s == 0:
        x, y = 0.3127, 0.3290
    else:
        x, y = X / s, Y / s

    client.publish(TOPIC_SET, json.dumps({"color": {"x": round(x, 4), "y": round(y, 4)}}))

    # Save RGB to state so GET /lamp/state returns them
    last_state["color_r"] = r
    last_state["color_g"] = g
    last_state["color_b"] = b

    return {"ok": True, "color": {"x": x, "y": y}}

@app.post("/lamp/color_temp")
def set_color_temp(body: ColorTempBody):
    v = max(153, min(500, body.value))
    client.publish(TOPIC_SET, json.dumps({"color_temp": v}))
    last_state["color_temp"] = v
    return {"ok": True, "color_temp": v}

@app.get("/lamp/state")
def lamp_state():
    state = last_state.copy()

    state.setdefault("brightness", 0)
    state.setdefault("color_temp", 250)
    state.setdefault("color_r", 255)
    state.setdefault("color_g", 255)
    state.setdefault("color_b", 255)
    print(f"STATE REQUESTED, returning: {state}")
    return {"ok": True, "state": state}