import yaml
import os
import pathlib
from ..config import TRAIN_DATASET, TRAIN_LOG
from ..services import get_train_data, get_labels
from .general import convert_pdf_bytes_to_image_arrays
import requests
import json
import cv2

def prepare_data(train_cfg, train_txt, val_txt, id):
    BASE_DIR = os.getcwd()
    LOG_SPACE = os.path.join(BASE_DIR, TRAIN_LOG)
    if not os.path.isdir(LOG_SPACE):
        os.makedirs(LOG_SPACE)
    # print(LOG_SPACE)
    train_file = os.path.join(LOG_SPACE,"train_{}.txt".format(id))
    val_file = os.path.join(LOG_SPACE,"val_{}.txt".format(id)) #"logs/val_{}.txt".format(id)
    dataset = os.path.join(LOG_SPACE,"dataset_{}.yaml".format(id)) #"logs/dataset_{}.yaml".format(id)
    
    f = open(train_file,'w')
    f.write(train_txt)
    f.close()
    
    f = open(val_file,'w')
    f.write(val_txt)
    f.close()
    
    dataset_cfg = train_cfg
    dataset_cfg["train"] = os.path.abspath(train_file)
    dataset_cfg["val"] = os.path.abspath(val_file)
    with open(dataset, 'w') as yaml_file:
        yaml.dump(dataset_cfg, yaml_file, default_flow_style=False)
        
    return dataset, train_file, val_file

def convert_label(label, train_labels):
    try:
        if len(train_labels) >=0  and label != {}:
            dst_label = ""
            for lb in label["label"]:
                cx = lb["x"] + lb["width"]/2
                cy = lb["y"] + lb["height"]/2
                w = lb["width"]
                h = lb["height"]
                if lb['label_id'] in train_labels:
                    class_id = train_labels.index(lb['label_id'])
                else:
                    continue
                line = "{} {:.6f} {:.6f} {:.6f} {:.6f}\n".format(class_id, cx, cy, w, h)
                dst_label+= line
            return dst_label
        
        return None 
    except:
        return None



def preprocess_data(form_code, train_session):
    try:
        
        BASE_DIR = os.getcwd()
        WORKSPACE = os.path.join(BASE_DIR,TRAIN_DATASET)
        if not os.path.isdir(WORKSPACE):
            os.makedirs(WORKSPACE)
        work_dir = os.path.join(WORKSPACE,train_session)
        if not os.path.isdir(work_dir):
            os.mkdir(work_dir)
        raw_dir = os.path.join(work_dir,"raws")
        if not os.path.isdir(raw_dir):
            os.mkdir(raw_dir)
        img_dir = os.path.join(work_dir,"images")
        if not os.path.isdir(img_dir):
            os.mkdir(img_dir)
        label_dir = os.path.join(work_dir,"labels")
        if not os.path.isdir(label_dir):
            os.mkdir(label_dir)
        
        data = get_train_data(form_code)
        labels = get_labels(form_code)
        
        train_labels = []
        train_cfg = {}
        train_txt = ''
        val_txt = ''
        nc = 0
        
        for label in labels:
            train_labels.append(label["code"])
            nc+=1
        train_cfg["nc"] = nc
        train_cfg["names"] = train_labels
        
        count = 0
        for dt in data:
            try:
                url = dt["url"]
                label = json.loads(dt["area"])
                if label == {}:
                    continue
                extension = pathlib.Path(url).suffix
                if extension in [".pdf",".jpg",'.png']:
                    response = requests.get(url, stream=True)
                    bytedata = response.content
                    if extension == ".pdf":
                        raw_name = os.path.join(raw_dir,str(count).zfill(5)+'.pdf')
                        img_name = os.path.join(img_dir,str(count).zfill(5)+'.jpg')
                        label_name = os.path.join(label_dir,str(count).zfill(5)+'.txt')
                        f = open(raw_name,"wb")
                        f.write(bytedata)
                        np_arrray = convert_pdf_bytes_to_image_arrays(bytedata)
                        cv2.imwrite(img_name, np_arrray)
                        
                    else:
                        raw_name = os.path.join(raw_dir,str(count).zfill(5)+extension)
                        f = open(raw_name,"wb")
                        f.write(bytedata)
                        img_name = os.path.join(img_dir,str(count).zfill(5)+extension)
                        fi = open(img_name,"wb")
                        fi.write(bytedata)
                        label_name = os.path.join(label_dir,str(count).zfill(5)+'.txt')
                    label_content = convert_label(label=label, train_labels=train_labels)
                    if label_content is not None:
                        lf = open(label_name,'w')
                        lf.write(label_content)
                    else:
                        continue
                    train_txt += img_name+'\n'
                    val_txt += img_name+'\n'
                    count=count+1
                    
            except:
                pass
        return train_txt, val_txt, train_cfg, WORKSPACE
    except:
        return None, None, None, None