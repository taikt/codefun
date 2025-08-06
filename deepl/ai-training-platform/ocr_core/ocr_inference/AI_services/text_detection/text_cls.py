# 3,48,192


import onnxruntime
import math
import cv2
import numpy as np

class TextAngleDetector():
    def __init__(self, model_path):
        sess = onnxruntime.InferenceSession(model_path, providers=["CPUExecutionProvider"])
        
        self.predictor, self.input_tensor, self.output_tensors = sess, sess.get_inputs()[0], None
        self.cls_image_shape = (3,48,192)

    def resize_norm_img(self, img):
        imgC, imgH, imgW = self.cls_image_shape
        h = img.shape[0]
        w = img.shape[1]
        ratio = w / float(h)
        if math.ceil(imgH * ratio) > imgW:
            resized_w = imgW
        else:
            resized_w = int(math.ceil(imgH * ratio))
        resized_image = cv2.resize(img, (resized_w, imgH))
        resized_image = resized_image.astype("float32")
        if self.cls_image_shape[0] == 1:
            resized_image = resized_image / 255
            resized_image = resized_image[np.newaxis, :]
        else:
            resized_image = resized_image.transpose((2, 0, 1)) / 255
        resized_image -= 0.5
        resized_image /= 0.5
        padding_im = np.zeros((imgC, imgH, imgW), dtype=np.float32)
        padding_im[:, :, 0:resized_w] = resized_image
        return padding_im
    
    def __call__(self, img):
        tensor = self.resize_norm_img(img)
        input_dict = {}
        input_dict[self.input_tensor.name] = np.expand_dims(tensor,axis=0)
        outputs = self.predictor.run(self.output_tensors, input_dict)
        # print(outputs)
        return outputs[0]

# if __name__ == '__main__':
#     model = TextAngleDetector('text_cls.onnx')
#     img = cv2.imread('test.png')
#     a = model(img)
#     print(a)