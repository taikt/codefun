#!/usr/bin/python
# encoding: utf-8

import torch
import torch.nn as nn
from torch.autograd import Variable
import collections
import numpy as np
# from PIL import Image
from torchvision.transforms import ToTensor, Normalize
import math
import cv2
mean = [0.485, 0.456, 0.406]
std = [0.229, 0.224, 0.225]

def resize(w, h, expected_height, image_min_width, image_max_width):
    new_w = int(expected_height * float(w) / float(h))
    round_to = 10
    new_w = math.ceil(new_w/round_to)*round_to
    new_w = max(new_w, image_min_width)
    new_w = min(new_w, image_max_width)

    return new_w, expected_height

def process_image(image, image_height, image_min_width, image_max_width):
    img = image.convert('RGB')

    w, h = img.size
    new_w, image_height = resize(w, h, image_height, image_min_width, image_max_width)

    img = img.resize((new_w, image_height), Image.ANTIALIAS)

    img = np.asarray(img).transpose(2,0, 1)
    img = img/255
    return img

# import cv2
# import numpy as np
# import torch
# from torchvision.transforms import ToTensor

def resizePadding(img, width, height):
    desired_w, desired_h = width, height
    img_h, img_w, _ = img.shape
    ratio = 1.0 * img_w / img_h
    new_w = int(desired_h * ratio)
    new_w = new_w if desired_w is None else min(desired_w, new_w)
    img_resized = cv2.resize(img, (new_w, desired_h), interpolation=cv2.INTER_LANCZOS4)

    if desired_w is not None and desired_w > new_w:
        padding_width = desired_w - new_w
        img_padded = cv2.copyMakeBorder(img_resized, 0, 0, 0, padding_width, cv2.BORDER_CONSTANT, value=(255, 255, 255))
    else:
        img_padded = img_resized

    img_normalized = img_padded.astype(np.float32) / 255.0
    img_tensor = torch.from_numpy(np.transpose(img_normalized, (2, 0, 1)))

    return img_tensor



# def resizePadding(img, width, height):
#     desired_w, desired_h = width, height  # (width, height)
#     img_w, img_h = img.size  # old_size[0] is in (width, height) format
#     ratio = 1.0 * img_w / img_h
#     new_w = int(desired_h * ratio)
#     new_w = new_w if desired_w == None else min(desired_w, new_w)
#     img = img.resize((new_w, desired_h), Image.LANCZOS)

#     # padding image
#     if desired_w != None and desired_w > new_w:
#         new_img = Image.new("RGB", (desired_w, desired_h), color=(255,255,255))
#         new_img.paste(img, (0, 0))
#         img = new_img
#     img = np.array(img)/255.0
#     img = ToTensor()(img)
#     # img = Normalize(mean, std)(img)
#     return img


def maxWidth(sizes, height):
    ws = [int(height * (1.0 * size[0] / size[1])) for size in sizes]
    maxw = max(ws)

    return maxw


class strLabelConverter(object):
    """Convert between str and label.

    NOTE:
        Insert `blank` to the alphabet for CTC.

    Args:
        alphabet (str): set of the possible characters.
        ignore_case (bool, default=True): whether or not to ignore all of the case.
    """

    def __init__(self, alphabet, ignore_case=True):
        self._ignore_case = ignore_case
        if self._ignore_case:
            alphabet = alphabet.lower()
        self.alphabet = alphabet + '-'  # for `-1` index

        self.dict = {}
        for i, char in enumerate(alphabet):
            # NOTE: 0 is reserved for 'blank' required by wrap_ctc
            self.dict[char] = i + 1

    def encode(self, text):
        """Support batch or single str.

        Args:
            text (str or list of str): texts to convert.

        Returns:
            torch.IntTensor [length_0 + length_1 + ... length_{n - 1}]: encoded texts.
            torch.IntTensor [n]: length of each text.
        """
        if isinstance(text, str):
            text = [
                self.dict[char.lower() if self._ignore_case else char]
                for char in text
            ]
            length = [len(text)]
        elif isinstance(text, collections.Iterable):
            length = [len(s) for s in text]
            text = ''.join(text)
            text, _ = self.encode(text)

        return (torch.IntTensor(text), torch.IntTensor(length))

    def decode(self, t, length, raw=False):
        """Decode encoded texts back into strs.

        Args:
            torch.IntTensor [length_0 + length_1 + ... length_{n - 1}]: encoded texts.
            torch.IntTensor [n]: length of each text.

        Raises:
            AssertionError: when the texts and its length does not match.

        Returns:
            text (str or list of str): texts to convert.
        """
        if length.numel() == 1:
            length = length[0]
            assert t.numel() == length, "text with length: {} does not match declared length: {}".format(t.numel(),
                                                                                                         length)
            if raw:
                return ''.join([self.alphabet[i - 1] for i in t])
            else:
                char_list = []
                for i in range(length):
                    if t[i] != 0 and (not (i > 0 and t[i - 1] == t[i])):
                        char_list.append(self.alphabet[t[i] - 1])
                return ''.join(char_list)
        else:
            # batch mode
            assert t.numel() == length.sum(), "texts with length: {} does not match declared length: {}".format(
                t.numel(), length.sum())
            texts = []
            index = 0
            for i in range(length.numel()):
                l = length[i]
                texts.append(
                    self.decode(
                        t[index:index + l], torch.IntTensor([l]), raw=raw))
                index += l
            return texts


class averager(object):
    """Compute average for `torch.Variable` and `torch.Tensor`. """

    def __init__(self):
        self.reset()

    def add(self, v):
        if type(v) is list:
            self.n_count += len(v)
            self.sum += sum(v)
        else:
            self.n_count += 1
            self.sum += v

    def reset(self):
        self.n_count = 0
        self.sum = 0

    def val(self):
        res = 0
        if self.n_count != 0:
            res = self.sum / float(self.n_count)
        return res


def oneHot(v, v_length, nc):
    batchSize = v_length.size(0)
    maxLength = v_length.max()
    v_onehot = torch.FloatTensor(batchSize, maxLength, nc).fill_(0)
    acc = 0
    for i in range(batchSize):
        length = v_length[i]
        label = v[acc:acc + length].view(-1, 1).long()
        v_onehot[i, :length].scatter_(1, label, 1.0)
        acc += length
    return v_onehot


def loadData(v, data):
    v.resize_(data.size()).copy_(data)


def prettyPrint(v):
    print('Size {0}, Type: {1}'.format(str(v.size()), v.data.type()))
    print('| Max: %f | Min: %f | Mean: %f' % (v.max().data[0], v.min().data[0],
                                              v.mean().data[0]))


def assureRatio(img):
    """Ensure imgH <= imgW."""
    b, c, h, w = img.size()
    if h > w:
        main = nn.UpsamplingBilinear2d(size=(h, h), scale_factor=None)
        img = main(img)
    return img


# def cer_loss_one_image(sim_pred, label):
#     loss = Levenshtein.distance(sim_pred, label) * 1.0 / max(len(sim_pred), len(label))
#     return loss


def cer_loss(sim_preds, labels):
    losses = []
    for i in range(len(sim_preds)):
        pred = sim_preds[i]
        text = labels[i]

        loss = cer_loss_one_image(pred, text)
        losses.append(loss)
    return losses