import cv2
import numpy as np
import requests
import serial
import time
from tensorflow.keras.models import load_model
from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.camera import Camera
from kivy.uix.button import Button
from kivy.uix.label import Label
from kivy.clock import Clock
from kivy.uix.scrollview import ScrollView
from kivy.core.window import Window

COM = "COM13"

# Nutrition API function
def get_nutrition(fruit_name):
    api_key = "vEdCgaQVNnG1xaTSzmsPDLDMm5iwEmrGolxPzlyd"  # Replace with your USDA API key
    url = "https://api.nal.usda.gov/fdc/v1/foods/search"
    params = {
        "query": fruit_name,
        "api_key": api_key,
        "pageSize": 1
    }
    try:
        response = requests.get(url, params=params, timeout=5)
        if response.status_code == 200:
            data = response.json()
            if data.get("foods"):
                nutrients = data["foods"][0].get("foodNutrients", [])
                nutrition_info = {
                    nutrient.get("nutrientName", "Unknown"): nutrient.get("value", 0) for nutrient in nutrients
                }
                return nutrition_info
            return {"error": "No data found."}
        return {"error": f"API request failed: {response.status_code}"}
    except requests.RequestException as e:
        return {"error": f"API error: {str(e)}"}

# Recipes API function
def get_recipes(fruit_name):
    api_key = "c9becf792f5648968445e5f885b12401"  # Replace with your Spoonacular API key
    uri = "https://api.spoonacular.com/recipes/complexSearch"
    params = {
        "query": fruit_name,
        "apiKey": api_key,
        "number": 8
    }
    try:
        response = requests.get(uri, params=params, timeout=5)
        if response.status_code == 200:
            recipes = response.json().get("results", [])
            return [recipe.get("title", "Unknown") for recipe in recipes]
        return ["No recipes found."]
    except requests.RequestException as e:
        return [f"API error: {str(e)}"]

# Load ML model and class indices
try:
    model = load_model('fruit_recognition_model.h5')
except Exception as e:
    print(f"Error loading model: {str(e)}")
    model = None

fruit_classes = {
    0: "Apple",
    1: "Garlic",
    2: "Ginger",
    3: "Onion",
    4: "Potato"
}

class FruitDetectionApp(App):
    def __init__(self, **kwargs):
        super(FruitDetectionApp, self).__init__(**kwargs)
        self.current_fruit = None
        self.last_fruit = None
        self.last_sent_fruit = None
        self.nutrition_cache = {}
        self.recipes_cache = {}
        self.serial_port = None
        self.serial_connected = False
        self.available_cameras = []
        self.current_camera_index = 0
        self.detect_available_cameras()
        self.try_connect_serial()

    def detect_available_cameras(self):
        """Detect available camera indices."""
        self.available_cameras = []
        for i in range(3):  # Check indices 0, 1, 2
            cap = cv2.VideoCapture(i)
            if cap.isOpened():
                self.available_cameras.append(i)
                cap.release()
        if not self.available_cameras:
            self.available_cameras = [0]  # Default to 0 if no cameras found

    def try_connect_serial(self):
        """Attempt to connect to the serial port."""
        try:
            if self.serial_port and self.serial_port.is_open:
                self.serial_port.close()
            self.serial_port = serial.Serial(COM, 9600, timeout=1)
            time.sleep(2)  # Wait for connection to stabilize
            self.serial_connected = True
            if hasattr(self, 'serial_label'):
                self.serial_label.text = "Serial: Connected"
        except serial.SerialException:
            self.serial_connected = False
            if hasattr(self, 'serial_label'):
                self.serial_label.text = "Serial: Not Connected"

    def start_serial(self, instance):
        """Start the serial connection."""
        self.try_connect_serial()

    def stop_serial(self, instance):
        """Stop the serial connection."""
        try:
            if self.serial_port and self.serial_port.is_open:
                self.serial_port.close()
                self.serial_connected = False
                self.serial_label.text = "Serial: Not Connected"
        except serial.SerialException:
            self.serial_label.text = "Serial: Error Closing"

    def cycle_camera(self, instance):
        """Cycle to the next available camera index."""
        if not self.available_cameras:
            self.camera_label.text = "Camera: None Available"
            return
        self.current_camera_index = (self.current_camera_index + 1) % len(self.available_cameras)
        new_index = self.available_cameras[self.current_camera_index]
        self.camera.index = new_index
        self.camera_label.text = f"Camera: Index {new_index}"

    def build(self):
        Window.size = (720, 1280)
        Window.clearcolor = (0.94, 0.94, 0.94, 1)

        layout = BoxLayout(orientation='vertical', padding=15, spacing=10)

        # Initialize camera with the first available index
        initial_camera_index = self.available_cameras[0] if self.available_cameras else 0
        self.camera = Camera(
            resolution=(640, 480),
            play=False,
            size_hint=(1, 0.35),
            index=initial_camera_index
        )

        self.label = Label(
            text="Press Start to Detect",
            size_hint=(1, 0.05),
            font_size='24sp',
            bold=True,
            color=(0.2, 0.2, 0.2, 1),
            halign='center',
            valign='middle',
            text_size=(Window.width - 30, None)
        )

        self.last_fruit_label = Label(
            text="Last Detected: None",
            size_hint=(1, 0.05),
            font_size='20sp',
            color=(0.2, 0.2, 0.2, 1),
            halign='center',
            valign='middle',
            text_size=(Window.width - 30, None)
        )

        self.last_sent_label = Label(
            text="Last Sent to Serial: None",
            size_hint=(1, 0.05),
            font_size='20sp',
            color=(0.2, 0.2, 0.2, 1),
            halign='center',
            valign='middle',
            text_size=(Window.width - 30, None)
        )

        self.serial_label = Label(
            text="Serial: " + ("Connected" if self.serial_connected else "Not Connected"),
            size_hint=(1, 0.05),
            font_size='20sp',
            color=(0.2, 0.2, 0.2, 1),
            halign='center',
            valign='middle',
            text_size=(Window.width - 30, None)
        )

        self.camera_label = Label(
            text=f"Camera: Index {initial_camera_index}",
            size_hint=(1, 0.05),
            font_size='20sp',
            color=(0.2, 0.2, 0.2, 1),
            halign='center',
            valign='middle',
            text_size=(Window.width - 30, None)
        )

        info_layout = BoxLayout(orientation='vertical', size_hint=(1, 0.3))

        nutrition_container = BoxLayout(orientation='vertical', size_hint=(1, 0.5))
        nutrition_header = Label(
            text="Nutrition Info",
            size_hint=(1, 0.1),
            font_size='20sp',
            bold=True,
            color=(0.172, 0.471, 0.451, 1)
        )
        nutrition_scroll = ScrollView(do_scroll_x=False, size_hint=(1, 0.9))
        self.nutrition_label = Label(
            text="",
            size_hint_y=None,
            font_size='18sp',
            color=(0.2, 0.2, 0.2, 1),
            halign='left',
            valign='top',
            text_size=(Window.width - 60, None),
            padding=(10, 10)
        )
        self.nutrition_label.bind(texture_size=lambda instance, value: setattr(instance, 'height', value[1] + 20))
        nutrition_scroll.add_widget(self.nutrition_label)
        nutrition_container.add_widget(nutrition_header)
        nutrition_container.add_widget(nutrition_scroll)

        recipes_container = BoxLayout(orientation='vertical', size_hint=(1, 0.5))
        recipes_header = Label(
            text="Recipes",
            size_hint=(1, 0.1),
            font_size='20sp',
            bold=True,
            color=(0.172, 0.471, 0.451, 1)
        )
        recipes_scroll = ScrollView(do_scroll_x=False, size_hint=(1, 0.9))
        self.recipes_label = Label(
            text="",
            size_hint_y=None,
            font_size='18sp',
            color=(0.2, 0.2, 0.2, 1),
            halign='left',
            valign='top',
            text_size=(Window.width - 60, None),
            padding=(10, 10)
        )
        self.recipes_label.bind(texture_size=lambda instance, value: setattr(instance, 'height', value[1] + 20))
        recipes_scroll.add_widget(self.recipes_label)
        recipes_container.add_widget(recipes_header)
        recipes_container.add_widget(recipes_scroll)

        info_layout.add_widget(nutrition_container)
        info_layout.add_widget(recipes_container)

        # Button layout for Start/Stop, Camera, and Serial controls
        button_layout = BoxLayout(orientation='horizontal', size_hint=(1, 0.1), spacing=10)

        self.btn = Button(
            text="Start",
            size_hint=(0.33, 1),
            background_color=(0.172, 0.471, 0.451, 1),
            background_normal='',
            font_size='22sp',
            bold=True,
            color=(1, 1, 1, 1)
        )
        self.btn.bind(on_press=self.toggle_camera)

        self.camera_btn = Button(
            text="Cycle Camera",
            size_hint=(0.33, 1),
            background_color=(0.172, 0.471, 0.451, 1),
            background_normal='',
            font_size='22sp',
            bold=True,
            color=(1, 1, 1, 1)
        )
        self.camera_btn.bind(on_press=self.cycle_camera)

        self.serial_start_btn = Button(
            text="Start Serial",
            size_hint=(0.33, 1),
            background_color=(0.172, 0.471, 0.451, 1),
            background_normal='',
            font_size='22sp',
            bold=True,
            color=(1, 1, 1, 1)
        )
        self.serial_start_btn.bind(on_press=self.start_serial)

        self.serial_stop_btn = Button(
            text="Stop Serial",
            size_hint=(0.33, 1),
            background_color=(0.867, 0.271, 0.271, 1),
            background_normal='',
            font_size='22sp',
            bold=True,
            color=(1, 1, 1, 1)
        )
        self.serial_stop_btn.bind(on_press=self.stop_serial)

        button_layout.add_widget(self.btn)
        button_layout.add_widget(self.camera_btn)
        button_layout.add_widget(self.serial_start_btn)
        button_layout.add_widget(self.serial_stop_btn)

        layout.add_widget(self.camera)
        layout.add_widget(self.label)
        layout.add_widget(self.last_fruit_label)
        layout.add_widget(self.last_sent_label)
        layout.add_widget(self.serial_label)
        layout.add_widget(self.camera_label)
        layout.add_widget(info_layout)
        layout.add_widget(button_layout)

        Window.bind(on_resize=self.update_text_size)
        Clock.schedule_interval(self.check_serial, 10)  # Check serial every 10 seconds

        return layout

    def update_text_size(self, instance, width, height):
        self.nutrition_label.text_size = (width - 60, None)
        self.recipes_label.text_size = (width - 60, None)
        self.label.text_size = (width - 30, None)
        self.last_fruit_label.text_size = (width - 30, None)
        self.last_sent_label.text_size = (width - 30, None)
        self.serial_label.text_size = (width - 30, None)
        self.camera_label.text_size = (width - 30, None)

    def check_serial(self, dt):
        if not self.serial_connected:
            self.try_connect_serial()

    def toggle_camera(self, instance):
        if not self.camera.play:
            self.camera.play = True
            self.btn.text = "Stop"
            self.btn.background_color = (0.867, 0.271, 0.271, 1)
            Clock.schedule_interval(self.detect_fruit, 2)  # Reduced frequency to 0.5 Hz
        else:
            self.camera.play = False
            self.btn.text = "Start"
            self.btn.background_color = (0.172, 0.471, 0.451, 1)
            Clock.unschedule(self.detect_fruit)

    def detect_fruit(self, dt):
        if not model:
            self.label.text = "Model not loaded!"
            return
        texture = self.camera.texture
        if not texture:
            self.label.text = "Camera not available!"
            return
        try:
            frame = np.frombuffer(texture.pixels, dtype=np.uint8).reshape((texture.height, texture.width, 4))
            frame = cv2.cvtColor(frame, cv2.COLOR_RGBA2RGB)
            resized = cv2.resize(frame, (100, 100))
            normalized = resized / 255.0
            input_tensor = np.expand_dims(normalized, axis=0)

            predictions = model.predict(input_tensor, verbose=0)
            class_idx = np.argmax(predictions)
            confidence = np.max(predictions)
            fruit_name = fruit_classes.get(class_idx, "Unknown")
            if confidence < 0.95:
                self.label.text = f"{fruit_name} ({confidence:.2f}) - Low Confidence"
                return
            self.label.text = f"{fruit_name} ({confidence:.2f})"

            if fruit_name != self.current_fruit:
                self.last_fruit = self.current_fruit
                self.current_fruit = fruit_name
                self.last_fruit_label.text = f"Last Detected: {self.last_fruit if self.last_fruit else 'None'}"

                if fruit_name in self.nutrition_cache:
                    nutrition_text = self.nutrition_cache[fruit_name]
                else:
                    nutrition = get_nutrition(fruit_name)
                    nutrition_text = "\n".join([f"{k}: {v}" for k, v in list(nutrition.items())[:5]]) if not isinstance(nutrition, dict) or "error" not in nutrition else nutrition.get("error", "Not Found")
                    self.nutrition_cache[fruit_name] = nutrition_text
                self.nutrition_label.text = nutrition_text

                if fruit_name in self.recipes_cache:
                    recipes_text = self.recipes_cache[fruit_name]
                else:
                    recipes = get_recipes(fruit_name)
                    recipes_text = "\n".join(recipes[:5]) if recipes else "Not Found"
                    self.recipes_cache[fruit_name] = recipes_text
                self.recipes_label.text = recipes_text

                if fruit_name in ["Ginger", "Onion", "Potato"] and self.serial_connected:
                    try:
                        fruit_name_cap = fruit_name[0].upper() + fruit_name[1:].lower()  # Capitalize for Arduino
                        self.serial_port.write((fruit_name_cap + '\n').encode())
                        self.last_sent_fruit = fruit_name_cap
                        self.last_sent_label.text = f"Last Sent to Serial: {self.last_sent_fruit}"
                    except serial.SerialException:
                        self.serial_label.text = "Serial: Error Sending Data"
                        self.serial_connected = False
                        self.try_connect_serial()
        except Exception as e:
            self.label.text = f"Error: {str(e)}"

    def on_stop(self):
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()

if __name__ == "__main__":
    FruitDetectionApp().run()