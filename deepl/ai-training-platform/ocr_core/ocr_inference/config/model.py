import os
OCR_WEIGHT_PATH = os.path.abspath("ocr_inference/weights/ocr.pth")
TEXT_DETECTION_WEIGHT_PATH = os.path.abspath("ocr_inference/weights/model_det_ch.onnx")
TEXT_ANGLE_WEIGHT_PATH = os.path.abspath("ocr_inference/weights/text_cls.onnx")
VOCAB_PATH = os.path.abspath("ocr_inference/weights/vocab.model")
FORM_PATH = os.path.abspath("ocr_inference/weights/forms")