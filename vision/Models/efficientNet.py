import os
import sys
import json
import numpy as np
import tensorflow as tf
from tensorflow.keras.applications import EfficientNetB0
from tensorflow.keras.applications.efficientnet import preprocess_input
from tensorflow.keras.preprocessing.image import load_img, img_to_array

try:
    model = EfficientNetB0(weights="imagenet")
except Exception as e:
    print(f"ERROR: CLS Model loading failed: {str(e)}")
    sys.exit(1)

IMG_SIZE = (224, 224)  # Must match EfficientNet's input size
PROBABILITY_THRESHOLD = 0.30  # Set minimum confidence level

valid_food_classes = {
    924, 925, 926, 927, 928, 929, 930, 931, 932, 933, 934, 935, 936, 937, 938,
    939, 940, 941, 942, 943, 944, 945, 946, 947, 948, 949, 950, 951, 952, 953,
    954, 955, 956, 957, 959, 960, 961, 962, 963, 964, 965, 966, 967, 968, 969
}

food_labels = {
    924: "guacamole", 925: "consomme", 927: "trifle",
    928: "ice_cream", 929: "ice_lolly", 930: "French_loaf", 931: "bagel",
    932: "pretzel", 933: "cheeseburger", 934: "hotdog", 935: "mashed_potato",
    936: "head_cabbage", 937: "broccoli", 938: "cauliflower", 939: "zucchini",
    940: "spaghetti_squash", 941: "acorn_squash", 942: "butternut_squash",
    943: "cucumber", 944: "artichoke", 945: "bell_pepper", 946: "cardoon",
    947: "mushroom", 948: "Granny_Smith", 949: "strawberry", 950: "orange",
    951: "lemon", 952: "fig", 953: "pineapple", 954: "banana", 955: "jackfruit",
    956: "custard_apple", 957: "pomegranate", 960: "chocolate_sauce",
    961: "dough", 962: "meat_loaf", 963: "pizza", 964: "potpie", 965: "burrito",
    966: "red_wine", 967: "espresso", 969: "eggnog"
}

def preprocess_image(image_path):
    if not os.path.exists(image_path):
        return "Image does not exist"

    try:
        img = load_img(image_path, target_size=IMG_SIZE)  # Load & resize
        img_array = img_to_array(img)  # Convert to numpy array
        img_array = preprocess_input(img_array)  # Normalize using EfficientNet preprocessing
        return np.expand_dims(img_array, axis=0)  # Add batch dimension
    except Exception as e:
        return str(e)

def classify_image(image_path):
    processed_image = preprocess_image(image_path)
    if isinstance(processed_image, str):  # This means an error message was returned
        return(f"ERROR: {processed_image}")
    try:
        preds = model.predict(processed_image, verbose=0)  # Get predictions
        top_class_index = np.argmax(preds)  # Get index of highest probability class
        top_probability = preds[0, top_class_index]  # Get the probability of the top class

        if top_class_index in valid_food_classes and top_probability >= PROBABILITY_THRESHOLD:
            return(F"CLASSIFICATION: {food_labels[top_class_index]}")
        else:
            return("INFO: Low confidence or not a valid food item")
    
    except Exception as e:
        return(f"ERROR: {str(e)}")