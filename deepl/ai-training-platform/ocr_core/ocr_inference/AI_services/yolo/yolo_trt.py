import argparse
from typing import Any

import cv2
import numpy as np
import onnxruntime as ort
import pycuda.driver as cuda
import pycuda.autoinit
import tensorrt as trt



# TRT_LOGGER = trt.Logger(trt.Logger.WARNING)
# import torch

# from ultralytics.yolo.utils import ROOT, yaml_load
# # from ultralytics.yolo.utils.checks import check_requirements, check_yaml
# import yaml 
# import re
# import time
# import threading

# NUM_CLASSES = 12
# NUM_KPS = 4




def static_resize(img, dh = 640, dw = 640):
    h,w,c = img.shape
    my_img = np.zeros((dh,dw,3),dtype=np.uint8)
    
    if h > w:
        nw = int(640*w/h)
        dim = (nw,640)
    else:
        nh = int(640*h/w)
        dim = (640,nh)
    rs_img = cv2.resize(img,dim, interpolation = cv2.INTER_AREA)
    rh, rw,_ = rs_img.shape
    # cv2.imwrite("aa.jpg",rs_img)
    my_img[0:rh,0:rw,:] = rs_img[:,:,:]
    # cv2.imwrite("a.jpg",my_img)
    return my_img
        

# def yaml_load(file='data.yaml', append_filename=False):
#     """
#     Load YAML data from a file.

#     Args:
#         file (str, optional): File name. Default is 'data.yaml'.
#         append_filename (bool): Add the YAML filename to the YAML dictionary. Default is False.

#     Returns:
#         dict: YAML data and file name.
#     """
#     with open(file, errors='ignore', encoding='utf-8') as f:
#         s = f.read()  # string

#         # Remove special characters
#         if not s.isprintable():
#             s = re.sub(r'[^\x09\x0A\x0D\x20-\x7E\x85\xA0-\uD7FF\uE000-\uFFFD\U00010000-\U0010ffff]+', '', s)

#         # Add YAML filename to dict and return
#         return {**yaml.safe_load(s), 'yaml_file': str(file)} if append_filename else yaml.safe_load(s)

# kps_color=[(255,0,0),(0,255,0),(0,0,255),(255,255,0),(255,0,0),(0,255,0),(0,0,255),(255,255,0),(255,0,0),(0,255,0),(0,0,255),(255,255,0),(255,0,0),(0,255,0),(0,0,255),(255,255,0),(255,255,0)]

class Yolov8PoseCard:

    def __init__(self, trt_model, confidence_thres, iou_thres, num_classes, num_kps):
        """
        Initializes an instance of the Yolov8 class.

        Args:
            onnx_model: Path to the ONNX model.
            input_image: Path to the input image.
            confidence_thres: Confidence threshold for filtering detections.
            iou_thres: IoU (Intersection over Union) threshold for non-maximum suppression.
        """
        # self.onnx_model = onnx_model
        # self.input_image = input_image
        self.confidence_thres = confidence_thres
        self.iou_thres = iou_thres
        self.num_classes = num_classes
        self.num_kps = num_kps
        self.num_obj = 8400
        self.input_width = 640
        self.input_height = 640
        self.input_shape = (1,3,640,640)
        self.output_shape = (1,self.num_obj*(self.num_kps*3+4+self.num_classes))
        # self.infer_mutex = threading.Lock()
        # self.thread_local = threading.local()

        logger = trt.Logger(trt.Logger.WARNING)
        self.runtime = trt.Runtime(logger)
        trt.init_libnvinfer_plugins(logger, namespace='')
        # engine_path = "model_fp16.trt"
        self.engine = self.load_engine(self.runtime, trt_model)

        self.stream = cuda.Stream()
        self.context = self.engine.create_execution_context() 
        self.d_input, self.d_output = self._allocate_device_memory()
        self.h_input, self.h_output = self._allocate_host_memory()

    def __del__(self):
        del self.engine
        del self.runtime
        del self.context
        del self.stream

    def load_engine(self,trt_runtime, engine_path):
        
        with open(engine_path, 'rb') as f:
            engine_data = f.read()
        return trt_runtime.deserialize_cuda_engine(engine_data)
    

        

    def _allocate_device_memory(self):
        input_size = trt.volume(self.input_shape) * trt.float32.itemsize
        output_size = trt.volume(self.output_shape) * trt.float32.itemsize
        d_input = cuda.mem_alloc(input_size)
        d_output = cuda.mem_alloc(output_size)
        return d_input, d_output

    def _allocate_host_memory(self):
        h_input = cuda.pagelocked_empty(trt.volume(self.input_shape), dtype=np.float32)
        h_output = cuda.pagelocked_empty(trt.volume(self.output_shape), dtype=np.float32)
        return h_input, h_output
    
    def do_inference(self,img_data):
        

        np.copyto(self.h_input, img_data.ravel())
        cuda.memcpy_htod_async(self.d_input, self.h_input, self.stream)
        self.context.execute_async(bindings=[int(self.d_input), int(self.d_output)], stream_handle=self.stream.handle)
        cuda.memcpy_dtoh_async(self.h_output, self.d_output, self.stream)
        self.stream.synchronize()
        return np.copy(self.h_output)
    

    def preprocess(self, img):
        """
        Preprocesses the input image before performing inference.

        Returns:
            image_data: Preprocessed image data ready for inference.
        """
        

        # Convert the image color space from BGR to RGB
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

        # Resize the image to match the input shape
        img = static_resize(img)
        # cv2.imwrite("a.jpg",img)
        # img = cv2.resize(img, (img_width, img_height))

        # Normalize the image data by dividing it by 255.0
        image_data = np.array(img) / 255.0

        # Transpose the image to have the channel dimension as the first dimension
        image_data = np.transpose(image_data, (2, 0, 1))  # Channel first

        # Expand the dimensions of the image data to match the expected input shape
        image_data = np.expand_dims(image_data, axis=0).astype(np.float32)

        # Return the preprocessed image data
        return image_data

    def postprocess(self, input_image, output):
        
        output = output.reshape(1,4+self.num_classes+self.num_kps*3,self.num_obj)
        outputs = np.transpose(np.squeeze(output[0]))
        rows = outputs.shape[0]

        # Lists to store the bounding boxes, scores, and class IDs of the detections
        boxes = []
        scores = []
        class_ids = []
        keypoints =[]

        h,w,_ = input_image.shape
        if h > w:
            # nw = int(640*w/h)
            factor = h / self.input_height
        else:

            factor = w / self.input_width
        
        
        # Iterate over each row in the outputs array
        # print(rows)
        for i in range(rows):
            # Extract the class scores from the current row
            classes_scores = outputs[i][4:4+self.num_classes] # 11 class: 4:15
            kps_raw_datas = outputs[i][4+self.num_classes:]
            max_score = np.amax(classes_scores)
            # print(max_score)

            # If the maximum score is above the confidence threshold
            if max_score >= self.confidence_thres:
                # print("here")
                # Get the class ID with the highest score
                class_id = np.argmax(classes_scores)

                # Extract the bounding box coordinates from the current row
                x, y, w, h = outputs[i][0], outputs[i][1], outputs[i][2], outputs[i][3]

                # Calculate the scaled coordinates of the bounding box
                left = int((x - w / 2) * factor)#x_factor)
                top = int((y - h / 2) * factor)#y_factor)
                width = int(w * factor)#x_factor)
                height = int(h * factor)#y_factor)

                # Add the class ID, score, and box coordinates to the respective lists
                class_ids.append(class_id)
                scores.append(max_score)
                boxes.append([left, top, width, height])
                kps = []
                for j in range (self.num_kps): # num kps
                    x = kps_raw_datas[j*3]*factor#x_factor
                    y = kps_raw_datas[j*3+1]*factor#y_factor
                    kps_conf = kps_raw_datas[j*3+2]
                    kps.append([x,y,kps_conf])
                keypoints.append(kps)
        # Apply non-maximum suppression to filter out overlapping bounding boxes
        indices = cv2.dnn.NMSBoxes(boxes, scores, self.confidence_thres, self.iou_thres)
        boxes_info = []
        # Iterate over the selected indices after non-maximum suppression
        for i in indices:
            # Get the box, score, and class ID corresponding to the index
            box_info = dict()
            box_info['box'] = boxes[i]
            box_info['score'] = scores[i]
            box_info['class_id'] = class_ids[i]
            box_info["keypoint"] = keypoints[i]
            boxes_info.append(box_info)

            # Draw the detection on the input image
            # self.draw_detections(input_image, box, score, class_id, keypoint)

        # Return the modified input image
        return boxes_info

    
    def __call__(self, img):
        # Preprocess the image data
        img_data = self.preprocess(img)
        # img_data = preprocess_with_dali([img])

        
        outputs = self.do_inference(img_data)
        

        # Perform post-processing on the outputs to obtain output image.
        boxes_info = self.postprocess(img, outputs)
        # print(boxes_info)
        return boxes_info

if __name__ == '__main__':
    # Create an argument parser to handle command-line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('--model', type=str, default='yolov8_pose.engine', help='Input your ONNX model.')
    # parser.add_argument('--img', type=str, default=str(ROOT / 'assets/bus.jpg'), help='Path to input image.')
    parser.add_argument('--img', type=str, default=str('test.jpg'), help='Path to input image.')
    parser.add_argument('--conf-thres', type=float, default=0.5, help='Confidence threshold')
    parser.add_argument('--iou-thres', type=float, default=0.5, help='NMS IoU threshold')
    args = parser.parse_args()

    # Check the requirements and select the appropriate backend (CPU or GPU)
    # check_requirements('onnxruntime-gpu' if torch.cuda.is_available() else 'onnxruntime')

    # Create an instance of the Yolov8 class with the specified arguments
    detection_model = Yolov8PoseCard(args.model,  args.conf_thres, args.iou_thres,12,4)

    # # Perform object detection and obtain the output image
    # t0 = time.time()
    
    
    img = cv2.imread(args.img)
    # print(detection_model(img))
    
    output_info = detection_model(img)
    print(output_info)
