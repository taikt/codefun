import cv2
import numpy as np
import onnxruntime as ort
import ast


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
        

class YoloInference:

    def __init__(self, onnx_model, confidence_thres=0.25, iou_thres=0.4):
        """
        Initializes an instance of the Yolov8 class.

        Args:
            onnx_model: Path to the ONNX model.
            input_image: Path to the input image.
            confidence_thres: Confidence threshold for filtering detections.
            iou_thres: IoU (Intersection over Union) threshold for non-maximum suppression.
        """
        self.onnx_model = onnx_model
        # self.input_image = input_image
        self.confidence_thres = confidence_thres
        self.iou_thres = iou_thres
        

        self.session = ort.InferenceSession(self.onnx_model, providers=['CPUExecutionProvider'])#,options = options) #'CUDAExecutionProvider', 

        # Get the model inputs
        self.metadata = self.session.get_modelmeta().custom_metadata_map
        self.names = ast.literal_eval(str(self.metadata['names']))
        
        self.num_classes = len(self.names.keys())
        
        self.model_inputs = self.session.get_inputs()

        # Store the shape of the input for later use
        input_shape = self.model_inputs[0].shape
        self.input_width = input_shape[2]
        self.input_height = input_shape[3]


    def preprocess(self, img):
        """
        Preprocesses the input image before performing inference.

        Returns:
            image_data: Preprocessed image data ready for inference.
        """
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

        img = static_resize(img)

        image_data = np.array(img) / 255.0

        image_data = np.transpose(image_data, (2, 0, 1))  # Channel first

        image_data = np.expand_dims(image_data, axis=0).astype(np.float32)

        return image_data

    def postprocess(self, input_image, output):

        
        outputs = np.transpose(np.squeeze(output[0]))
        rows = outputs.shape[0]

        # Lists to store the bounding boxes, scores, and class IDs of the detections
        boxes = []
        scores = []
        class_ids = []
        
        h,w,_ = input_image.shape
        if h > w:
            factor = h / self.input_height
        else:
            
            factor = w / self.input_width
        
        for i in range(rows):
            # Extract the class scores from the current row
            classes_scores = outputs[i][4:4+self.num_classes] # 11 class: 4:15
            max_score = np.amax(classes_scores)

            # If the maximum score is above the confidence threshold
            if max_score >= self.confidence_thres:
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
                
        # Apply non-maximum suppression to filter out overlapping bounding boxes
        indices = cv2.dnn.NMSBoxes(boxes, scores, self.confidence_thres, self.iou_thres)
        boxes_info = []
        # Iterate over the selected indices after non-maximum suppression
        for i in indices:
            # Get the box, score, and class ID corresponding to the index
            box_info = dict()
            box_info['box'] = boxes[i]
            box_info['score'] = scores[i]
            box_info['class_name'] = self.names[class_ids[i]]
            boxes_info.append(box_info)
        

        # Return the modified input image
        return boxes_info

    
    def __call__(self, img):
        # Preprocess the image data
        img_data = self.preprocess(img)

        outputs = self.session.run(None, {self.model_inputs[0].name: img_data})
        # Perform post-processing on the outputs to obtain output image.
        boxes_info = self.postprocess(img, outputs)
        return boxes_info
        

# if __name__ == '__main__':
#     yolo = YoloInference('test.onnx')
#     import cv2 
    
#     img = cv2.imread('test.jpg')
#     b = yolo(img)
#     print(b)