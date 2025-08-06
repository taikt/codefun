import os
import sys

__dir__ = os.path.dirname(os.path.abspath(__file__))
sys.path.append(__dir__)
sys.path.insert(0, os.path.abspath(os.path.join(__dir__, './')))

import torch
import numpy as np
# from pipeline.ca model.utils import resizePadding
from ..ocr_vgg.ocr_model import CRNNTRANS#_BUI
# from PIL import Image
from torch.nn.functional import softmax
import cv2
from PIL import Image

import sentencepiece as spm

class OCR():
    def __init__(self, path_ckpt,  vocab_path ,max_seq_length= 205, device ="cpu") -> None:
        super().__init__()
        self.device = torch.device(device)
        self.tokenizer = spm.SentencePieceProcessor(model_file=vocab_path)
        self.model = CRNNTRANS.OCR(self.tokenizer.get_piece_size()).to(self.device) #_BUI
        state_dict = torch.load(path_ckpt, map_location=device) 
        self.max_seq_length = max_seq_length
        self.model.load_state_dict(state_dict)  
        self.IMAGE_HEIGHT = 56
        self.IMAGE_WIDTH = 672
        self.sos_token = 2
        self.eos_token = 3
        self.model.eval()	

        
    # def resizePadding(self,img):
    #     # desired_w, desired_h = width, height
    #     img_h, img_w, _ = img.shape
    #     ratio = 1.0 * img_w / img_h
    #     new_w = int(self.IMAGE_HEIGHT * ratio)
    #     new_w = new_w if self.IMAGE_WIDTH  is None else min(self.IMAGE_WIDTH , new_w)
    #     img_resized = cv2.resize(img, (new_w, self.IMAGE_HEIGHT), interpolation=cv2.INTER_LANCZOS4)

    #     if self.IMAGE_WIDTH  is not None and self.IMAGE_WIDTH  > new_w:
    #         padding_width = self.IMAGE_WIDTH  - new_w
    #         img_padded = cv2.copyMakeBorder(img_resized, 0, 0, 0, padding_width, cv2.BORDER_CONSTANT, value=(255, 255, 255))
    #     else:
    #         img_padded = img_resized

    #     img_normalized = img_padded.astype(np.float32) / 255.0
    #     img_tensor = np.transpose(img_normalized, (2, 0, 1))#torch.from_numpy()

    #     return img_tensor

    def process_input(self,images):
        imgs = []
        for image in images:
            # cv2.imwrite('test/'+str(idx)+'.jpg',image)
            imgs.append(self.process_image(image))
        imgs = np.array(imgs) # = img[np.newaxis, ...]
        imgs = torch.FloatTensor(imgs).to(device=self.device) #.cuda()
        return imgs

    def process_image(self,image):
        image = Image.fromarray(image)
        img = image.convert('RGB')

        # w, h = img.size
        # new_w, image_height = resize(w, h, image_height, image_min_width, image_max_width)

        # img = img.resize((new_w, image_height), Image.ANTIALIAS)
        img = self.resizePadding(img)

        img = np.asarray(img).transpose(2, 0, 1)
        img = img/255.0
        return img

    def resizePadding(self,img):
        try:
            desired_w, desired_h = self.IMAGE_WIDTH, self.IMAGE_HEIGHT  # (width, height)
            img_w, img_h = img.size  # old_size[0] is in (width, height) format
            ratio = 1.0 * img_w / img_h
            new_w = int(desired_h * ratio)
            new_w = new_w if desired_w == None else min(desired_w, new_w)
            img = img.resize((new_w, desired_h), Image.LANCZOS)#Image.ANTIALIAS)

            # padding image
            if desired_w != None and desired_w > new_w:
                new_img = Image.new("RGB", (desired_w, desired_h), color=(255,255,255))
                new_img.paste(img, (0, 0))
                img = new_img
            # img = np.array(img)/255.0
            # img = ToTensor(img)#()
            # img = Normalize(mean, std)(img)
            return img
        except Exception as e:
            print(e)
            return Image.new("RGB", (desired_w, desired_h), color=(0,0,0))
        
    def __call__(self, images):
        len_img = len(images)
        input_imgs = self.process_input(images)
        src, _ = self.model.cnn(input_imgs)

        memory = self.model.transformer.forward_encoder(src)	

        translated_sentence = [[self.sos_token]*len_img]	
        max_length = 0	

        while max_length <= self.max_seq_length and not all(np.any(np.asarray(translated_sentence).T==self.eos_token, axis=1)):	
            tgt_inp = torch.LongTensor(translated_sentence).to(self.device)	
                
            output,_, memory = self.model.transformer.forward_decoder(tgt_inp, memory)	
            output = softmax(output, dim=-1)
            output = output.to(self.device)		 

            values, indices  = torch.topk(output, 5) # get position of 5 element having largest values

            indices = indices[:, -1, 0]	# get position of 1 element having the largest value
            indices = indices.tolist()
            
            translated_sentence.append(indices)	
            max_length += 1	
            del output	

        translated_sentence = np.asarray(translated_sentence).T
        translated_sentence = translated_sentence.tolist()
        # print(translated_sentence)
        translated_sentence = self.tokenizer.decode(translated_sentence)
        
        return translated_sentence

    def translate(self, img, len_img):#, max_seq_length=205, sos_token=2, eos_token=3):
        "data: BxCXHxW"	
        	
        with torch.no_grad():
            # self.model.cnn.cuda()	
            # 
            # img = img.half()
            src, _ = self.model.cnn(img)

            memory = self.model.transformer.forward_encoder(src)	

            translated_sentence = [[self.sos_token]*len_img]	
            max_length = 0	

            while max_length <= self.max_seq_length and not all(np.any(np.asarray(translated_sentence).T==self.eos_token, axis=1)):	
                tgt_inp = torch.LongTensor(translated_sentence).to(self.device)	
                    
                output,_, memory = self.model.transformer.forward_decoder(tgt_inp, memory)	
                output = softmax(output, dim=-1)
                output = output.to(self.device)		 

                values, indices  = torch.topk(output, 5) # get position of 5 element having largest values

                indices = indices[:, -1, 0]	# get position of 1 element having the largest value
                indices = indices.tolist()
                
                translated_sentence.append(indices)	
                max_length += 1	
                del output	

            translated_sentence = np.asarray(translated_sentence).T
            translated_sentence = translated_sentence[0].tolist()
            print(translated_sentence)
            translated_sentence = self.tokenizer.decode(translated_sentence)

        return translated_sentence

# path_ckpts =['last0909.pth']#["ckpt/mix_sent/mix_iter_387000.pth","mix_iter_2777000.pth", "mix_iter_435000.pth"]
# # path_ckpt ="mix_iter_2777000.pth"
# # path_ckpt ="mix_iter_435000.pth"
# for path_ckpt in path_ckpts:
#     ocr = OCR(path_ckpt)

#     # from natsort import natsorted
#     # from tqdm import tqdm
#     # import glob
#     # paths = natsorted(glob.glob("random_test/*"))


#     # for path in tqdm(paths):
#     #     img = Image.open(path).convert('RGB')
#     #     out = ocr.infer([img])
#     #     print(path, out)

#     path = "597.jpg"
#     text_img = cv2.imread(path)
#     text_img = cv2.cvtColor(text_img, cv2.COLOR_BGR2RGB)
#     im_pil = Image.fromarray(text_img)
#     import time
    
#     while True:
        
#         bathc_img = [im_pil]*1
#         t1 = time.time()
#         out = ocr.infer(bathc_img)
#         print(time.time()-t1)
#         print(path_ckpt, out)