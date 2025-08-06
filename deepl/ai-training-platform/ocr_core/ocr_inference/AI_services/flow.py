from ..config import OCR_WEIGHT_PATH, TEXT_DETECTION_WEIGHT_PATH, VOCAB_PATH, TEXT_ANGLE_WEIGHT_PATH
from .ocr_vgg import OCR
from .text_detection import TextDetector, TextAngleDetector
from .yolo import YoloInference
from ..utils import get_crop_image
import numpy as np
import os
# import yaml
import time
import cv2
from dotenv import load_dotenv
load_dotenv()

device = "cpu" if not os.environ.get('DEVICE') else os.environ.get('DEVICE')

ocr_model = OCR(OCR_WEIGHT_PATH, VOCAB_PATH, device=device)
text_detection_model = TextDetector(TEXT_DETECTION_WEIGHT_PATH)
text_cls_model = TextAngleDetector(TEXT_ANGLE_WEIGHT_PATH)


def ocr_line(img):
    h,w,c = img.shape
    dets = text_detection_model(img)
    dets = sorted(dets, key=lambda x: x[0][1])
    if len(dets) == 0:
        dets.append(np.array([[0, 0],
                              [w, 0],
                              [w, h],
                              [0,h]]))
    result = []
    for idx, det in enumerate(dets):
        kps = np.array(det).astype(np.float32)
        crop_text_img = get_crop_image(kps,img)
        lh,lw,_ = crop_text_img.shape
        is90 = False
        if lh > lw*1.2:
            is90 = True
        if is90:
            crop_text_img = cv2.rotate(crop_text_img, cv2.ROTATE_90_CLOCKWISE)
        direction_det = text_cls_model(crop_text_img)[0]
        
        if direction_det[1] > direction_det[0]:
            crop_text_img = cv2.rotate(crop_text_img, cv2.ROTATE_180)
        # cv2.imwrite("a{}.jpg".format(idx),crop_text_img)
        result.append(ocr_model([crop_text_img])[0])
        print(result) 
    return result

def ocr_by_form(img, inference_path):
    detect_model = YoloInference(inference_path)
    bboxes_info = detect_model(img)
    sorted_bboxes_info = sorted(bboxes_info, key=lambda x: x["box"][1])
    result = []
    for bbox in sorted_bboxes_info:
        x1 = int(bbox['box'][0])
        y1 = int(bbox['box'][1])
        x2 = int(bbox['box'][0]+bbox['box'][2])
        y2 = int(bbox['box'][1]+bbox['box'][3])
        crop_img = img[y1:y2,x1:x2,:]
        class_name = bbox['class_name']
        ocr_text = ocr_model([crop_img])[0]
        result.append({"key": class_name, "value": ocr_text, "conf": bbox["score"]})
        print(result)
    return result