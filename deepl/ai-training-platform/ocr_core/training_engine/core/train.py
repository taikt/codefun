from ultralytics import YOLO
from ..utils import preprocess_data, prepare_data, update_status, update_progress, update_accuracy, update_path
import os
import yaml
import argparse
import numpy as np
from ..config import *
from ..services import update_file
import shutil
# from ..views import update_accuracy, update_progress, update_status
from dotenv import load_dotenv
load_dotenv()


DEVICE_TRAIN = "cpu" if not os.environ.get('DEVICE_TRAIN') else os.environ.get('DEVICE_TRAIN')

def data_house_keeping(ws, dataset, trainfile, valfile, experiment_dir, id):
    if os.path.isdir(ws):
        shutil.rmtree(ws)
    if os.path.isfile(dataset):
        os.remove(dataset)
    if os.path.isfile(trainfile):
        os.remove(trainfile)
    if os.path.isfile(valfile):
        os.remove(valfile)
    model_dir = os.path.join(experiment_dir,str(id))
    if os.path.isdir(model_dir):
        shutil.rmtree(model_dir)
    return True

    
def train_model(form_code, id):
    try:
        # print("####################")
        update_status(id, "PREPARING")
        traintxt, valtxt, traincfg, ws = preprocess_data(form_code, id)
        update_status(id, "SETTING")
        dataset, trainfile, valfile = prepare_data(traincfg, traintxt, valtxt, id)
        print(dataset)
        def on_train_epoch_end(trainer):
            current_epoch = trainer.epoch
            all_epoch = trainer.epochs
            update_progress(id, current_epoch, all_epoch)
        
        update_status(id, "TRAINING")
        model = YOLO("yolo11s.pt")
        model.add_callback("on_train_epoch_end",on_train_epoch_end)
        
        BASE_DIR = os.getcwd()
        experiment_dir = os.path.join(BASE_DIR,TRAIN_EXPERIMENT)
        if not os.path.isdir(experiment_dir):
            os.makedirs(experiment_dir)

        res = model.train(
                        data=dataset,  # path to dataset YAML
                        epochs=300,
                        batch =8,
                        workers=4,
                        imgsz=640,  # training image size
                        device=DEVICE_TRAIN,
                        project = experiment_dir,
                        name = str(id)  # device to run on, i.e. device=0 or device=0,1,2,3 or device=cpu
                        )
        accuraccy = np.mean(res.maps)
        print("Accuracy", accuraccy)
        update_accuracy(id, float(accuraccy))
        
        model_path = os.path.join(experiment_dir,os.path.join(str(id),'weights/best.pt'))
        if not os.path.isfile(model_path):
            data_house_keeping(ws, dataset, trainfile, valfile, experiment_dir, id)
            update_status(id, "FAILED")
            return False
        dst_model_path = model_path.replace('best.pt',str(id)+'.pt')
        os.rename(model_path, dst_model_path)
        cvt_model = YOLO(dst_model_path)
        cvt_model.export(format="onnx")
        print("CONVERT TO ONNX DONE")
        dst_onnx = dst_model_path.replace('.pt','.onnx')
        minio_model_path = update_file(dst_model_path)
        if os.path.isfile(dst_onnx):
            print(dst_onnx)
        minio_inference_path = update_file(dst_onnx)
        print("MINIO SAVE", minio_model_path, minio_inference_path)
        if minio_model_path is not None:
            update_path(id, minio_model_path, minio_inference_path)
            update_status(id, "SUCCESS")
            update_progress(id, 300, 300)
            # data_house_keeping(ws, dataset, trainfile, valfile, experiment_dir, id)
            print("DATA HOUSE KEEPING DONE")
            return True
        data_house_keeping(ws, dataset, trainfile, valfile, experiment_dir, id)
        update_status(id, "FAILED")
        return False
    except Exception as e:
        print(e)
        data_house_keeping(ws, dataset, trainfile, valfile, experiment_dir, id)
        update_status(id, "FAILED")
        return False
        
