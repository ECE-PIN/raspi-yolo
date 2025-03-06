import easyocr
import re
import sys
import cv2
import numpy as np

try:
    reader = easyocr.Reader(['en'], gpu=False)
except Exception as e:
    raise(f"ERROR: OCR model loading failed: {str(e)}")

classification_keywords = {
    "milk", "cheese", "bread", "eggs", "yogurt", "butter", "juice", "meat", "chicken",
    "fish", "cereal", "pasta", "rice", "flour", "sugar", "salt", "pepper", "coffee",
    "tea", "honey", "jam", "peanut butter", "chocolate", "candy", "cookies", "crackers",
    "granola", "oats", "popcorn", "chips", "pancake mix", "syrup", "beans", "corn",
    "tomatoes", "tuna", "soup", "fruit", "vegetables", "meals", "pizza", "fries",
    "ice cream", "energy bars", "protein powder", "noodles", "ketchup", "mayonnaise",
    "mustard", "soy sauce", "hot sauce", "dressing", "oil", "vinegar", "pudding",
    "whipped cream", "sour cream", "tofu", "bacon"
}

plu_mapping = {
    # Apples
    4101: "Braeburn Apple",
    4106: "Cortland Apple",
    4131: "Fuji Apple",
    4129: "Fuji Apple (Small)",
    4193: "Fuji Apple (Large)",
    4135: "Gala Apple",
    4173: "Gala Apple (Large)",
    3283: "Honeycrisp Apple",
    3293: "Jazz Apple",
    4145: "Jonathan Apple",
    4017: "Granny Smith Apple",
    4019: "McIntosh Apple",
    4128: "Pink Lady Apple",
    4015: "Red Delicious Apple",
    4167: "Red Delicious Apple (Large)",
    4020: "Golden Delicious Apple",
    4021: "Golden Delicious Apple (Large)",
    
    # Avocados
    4046: "Hass Avocado",
    4225: "Large Hass Avocado",
    4223: "Florida Avocado",
    
    # Bananas
    4011: "Banana",
    4186: "Banana (Small)",
    4236: "Red Banana",
    
    # Bell Peppers
    4065: "Green Bell Pepper",
    4681: "Green Bell Pepper (Large)",
    4088: "Red Bell Pepper",
    4682: "Orange Bell Pepper",
    4680: "Yellow Bell Pepper",
    
    # Berries & Exotic Fruits
    4478: "Mangosteen",
    3037: "Mulberry",
    3039: "Gooseberry",
    
    # Citrus Fruits
    4053: "Lemon",
    4958: "Lemon (Large)",
    4048: "Lime",
    4305: "Lime (Large)",
    4012: "Navel Orange",
    3107: "Navel Orange (Large)",
    3108: "Valencia Orange",
    4014: "Valencia Orange (Small)",
    4388: "Valencia Orange (Large)",
    4381: "Blood Orange",
    3110: "Cara Cara Orange",
    4281: "Pink Grapefruit",
    4282: "Red Grapefruit",
    4294: "White Grapefruit",
    
    # Melons
    4050: "Cantaloupe",
    4319: "Cantaloupe (Large)",
    4032: "Seedless Watermelon",
    4031: "Watermelon with Seeds",
    4342: "Watermelon (Seedless)",
    
    # Pears
    4408: "Asian Pear",
    4416: "Green Anjou Pear",
    4417: "Red Anjou Pear",
    4409: "Bartlett Pear",
    4425: "Bartlett Pear (Large)",
    4413: "Bosc Pear",
    4418: "Bosc Pear (Large)",
    4414: "Comice Pear",
    4422: "Seckel Pear",
    
    # Mangoes
    4051: "Mango",
    4959: "Mango (Large)",
    4312: "Ataulfo (Honey) Mango",
    3621: "Green Mango",
    
    # Other Fruits
    4450: "Guava",
    4479: "Lychee",
    4959: "Fuyu Persimmon",
    4424: "Hachiya Persimmon",
    4235: "Plantain",
    4327: "Cherimoya",
    4444: "Brown Turkey Fig",
    3038: "Black Mission Fig",
    4306: "White Flesh Dragon Fruit",
    4314: "Red Flesh Dragon Fruit",
    4279: "Sapodilla",
    4489: "Tamarillo",
    4455: "Quince",
    3093: "Red Prickly Pear",
    3427: "Green Prickly Pear",
    4302: "Jackfruit",
    4309: "Rambutan",
    3040: "Durian",
    3132: "Longan",
    
    # Plums & Pomegranates
    4040: "Black Plum",
    4440: "Black Plum (Large)",
    4042: "Red Plum",
    4445: "Pluot Plum",
    4446: "Pomegranate",
    
    # Potatoes
    4072: "Russet Potato",
    4728: "Russet Potato (Large)",
    4727: "Gold (Yukon) Potato",
    4073: "Red Potato",
    4091: "Sweet Potato",
    4816: "Sweet Potato (Large)",
    4083: "White Potato",
    
    # Tomatoes
    4799: "Beefsteak Tomato",
    3423: "Heirloom Tomato",
    4087: "Roma Tomato",
    4664: "Vine Ripe Tomato",
    4665: "Cluster Tomato",
    3424: "Grape Tomato",
    
    # Other Vegetables
    4092: "Loose Carrots",
    4070: "Celery",
    4576: "Celery (Hearts)",
    4061: "Iceberg Lettuce",
    4060: "Broccoli",
    4067: "Zucchini",
    4755: "Zucchini (Large)",
    4785: "Butternut Squash",
    4779: "Spaghetti Squash",
    4511: "Cactus Pear",
    4505: "Breadfruit",
    3051: "Chayote",
    3050: "Jicama",
    4558: "Daikon Radish",
    4629: "Kohlrabi",
    3092: "Taro Root",
    4650: "Yuca Root",
    4256: "Starfruit (Carambola)"
}

def clean_text(text):
    """ Normalize and clean extracted OCR text while keeping numbers. """
    return re.sub(r'[^a-zA-Z0-9\s]', '', text).strip().lower()

def is_text_class(text):
    """ Check if extracted text belongs to a known classification. """
    words = clean_text(text).split()
    for word in words:
        if(len(word) == 4 and word.isdigit() and int(word) in plu_mapping):
            return {"type": "classification", "value": plu_mapping.get(int(word))}
        if len(word) > 2 and word in classification_keywords:
            return {"type": "classification", "value": word}  # Return food category

    return None

def preprocess_image(image_path):
    print("Preprocessing image")
    img = cv2.imread(image_path)

    if img is None:
        return("Image not found")
    
    # Ensure minimum width while maintaining aspect ratio
    min_width = 600  # Define minimum width
    height, width = img.shape[:2]
    
    if width < min_width:
        scale = min_width / width
        new_width = int(width * scale)
        new_height = int(height * scale)
        img = cv2.resize(img, (new_width, new_height), interpolation=cv2.INTER_CUBIC)

    # Convert to grayscale
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    # Apply Contrast Limited Adaptive Histogram Equalization (CLAHE)
    clahe = cv2.createCLAHE(clipLimit=3.0, tileGridSize=(8, 8))
    gray = clahe.apply(gray)

    # Use Morphological Gradient to highlight text regions
    kernel = np.ones((3, 3), np.uint8)
    grad = cv2.morphologyEx(gray, cv2.MORPH_GRADIENT, kernel)

    # Apply adaptive thresholding (handles variable lighting)
    binary = cv2.adaptiveThreshold(grad, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, 
                                   cv2.THRESH_BINARY, 11, 2)

    # Find text contours
    contours, _ = cv2.findContours(binary, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # Create a mask to extract text regions
    mask = np.zeros_like(gray)
    for cnt in contours:
        x, y, w, h = cv2.boundingRect(cnt)
        if w > 30 and h > 10:  # Ignore small non-text contours
            mask[y:y+h, x:x+w] = 255

    # Apply the mask
    processed = cv2.bitwise_and(gray, gray, mask=mask)

    print("Image Preprocessed")
    return processed  # Return improved image as a NumPy array
    
def extract_text_regions(image_path):
    """ First pass: Extract bounding boxes of text regions without reading the text. """
    print("Extracting text regions")
    
    processed_image = preprocess_image(image_path)
    if isinstance(processed_image, str):  # If an error message is returned
        return processed_image, None  # Return as a tuple

    try:
        # Run OCR to get bounding boxes only
        results = reader.readtext(processed_image, detail=1, paragraph=True, text_threshold=0.3)

        if not results:
            return "ERROR: No text detected by EasyOCR", None
        
        bounding_boxes = []
        for result in results:
            if not isinstance(result, (tuple, list)) or len(result) < 2:
                print(f"Skipping invalid result: {result}")  # Debug print
                continue  # Skip invalid results

            bbox, text = result[:2]  # Extract bounding box and text safely
            prob = result[2] if len(result) > 2 else 1.0  # Default confidence if missing
            
            # Print detected text for debugging
            print(f"Detected Text: '{text}' (Confidence: {prob})")

            if prob < 0.3:
                continue  # Skip low-confidence detections

            # Extract bounding box coordinates
            (top_left, top_right, bottom_right, bottom_left) = bbox
            x_min = int(min(top_left[0], bottom_left[0]))
            x_max = int(max(top_right[0], bottom_right[0]))
            y_min = int(min(top_left[1], top_right[1]))
            y_max = int(max(bottom_left[1], bottom_right[1]))

            bounding_boxes.append((x_min, y_min, x_max, y_max))

        print(f"Extracted {len(bounding_boxes)} text regions.")

        if not bounding_boxes:
            return "ERROR: No text regions found", None

        return bounding_boxes, processed_image  # Return a tuple

    except Exception as e:
        return f"ERROR: {str(e)}", None  # Return error string as tuple




def perform_ocr_with_zoom(image_path):
    """ Second pass: Run OCR on zoomed-in text regions. """
    print("Performing second pass OCR")

    bounding_boxes, processed_image = extract_text_regions(image_path)

    if isinstance(bounding_boxes, str):  # If it's an error message
        return bounding_boxes  # Return the error

    print(f"Extracted Bounding Boxes: {bounding_boxes}")

    refined_results = []

    for bbox in bounding_boxes:
        if len(bbox) != 4:
            print(f"Skipping invalid bounding box: {bbox}")
            continue

        x_min, y_min, x_max, y_max = bbox
        print(f"Processing bounding box: x_min={x_min}, y_min={y_min}, x_max={x_max}, y_max={y_max}")

        cropped_region = processed_image[y_min:y_max, x_min:x_max]

        if cropped_region.size == 0:
            print("Skipped: Empty cropped region")
            continue

        # Resize image for better OCR accuracy
        cropped_region = cv2.resize(cropped_region, None, fx=2, fy=2, interpolation=cv2.INTER_CUBIC)

        # Apply sharpening for better OCR
        sharpen_kernel = np.array([[-1, -1, -1], [-1, 9, -1], [-1, -1, -1]])
        cropped_region = cv2.filter2D(cropped_region, -1, sharpen_kernel)

        # Run OCR on the cropped image
        zoomed_texts = reader.readtext(cropped_region, detail=0, paragraph=True, text_threshold=0.7)

        print(f"Zoomed OCR Results: {zoomed_texts}")

        for zoomed_text in zoomed_texts:
            classification = is_text_class(zoomed_text)
            if classification:
                refined_results.append(classification)

    print("OCR performed on refined text regions")

    if refined_results:
        return "\n".join([f"CLASSIFICATION: {result['value']}" for result in refined_results])
    else:
        return "INFO: No valid classifications found"



def perform_ocr(image_path):
    print("Performing OCR")
    processed_image = preprocess_image(image_path)
    if isinstance(processed_image, str):
        return(f"ERROR: {processed_image}")

    try:
        print("Reading text");
        results = reader.readtext(processed_image, detail=0, paragraph=True, text_threshold=0.8)
        print("Text read");

        # Filter only valid classifications
        filtered_results = [is_text_class(text) for text in results if is_text_class(text)]

        print("OCR performed")
        if filtered_results:
            for result in filtered_results:
                return(f"CLASSIFICATION: {result['value']}")
        else:
            return("INFO: No valid classifications found")
    except Exception as e:
        return(f"ERROR: {str(e)}")
