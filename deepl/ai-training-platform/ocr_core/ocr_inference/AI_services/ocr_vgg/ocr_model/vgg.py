import torch
from torch import nn
from torchvision import models

class Vgg(nn.Module):
    def __init__(self, name, ss, ks, hidden, pretrained=True, dropout=0.5):
        super(Vgg, self).__init__()

        if name == 'vgg11_bn':
            cnn = models.vgg11_bn(pretrained=pretrained)
        elif name == 'vgg19_bn':
            cnn = models.vgg19_bn(pretrained=pretrained)

        pool_idx = 0

        for i, layer in enumerate(cnn.features):
            if isinstance(layer, torch.nn.MaxPool2d):
                cnn.features[i] = torch.nn.AvgPool2d(kernel_size=ks[pool_idx], stride=ss[pool_idx], padding=0)
                pool_idx += 1

        self.features = cnn.features
        self.dropout = nn.Dropout(dropout)
        self.last_conv_1x1 = nn.Conv2d(512, hidden, 1)
        #self.check = nn.Flatten(nn.Conv2d(512, hidden, 1))

        # self.flatten = nn.Flatten()
        # self.font_clasifier = nn.Linear(129024,31)
        self.classifier = nn.Sequential(
           nn.Conv2d(hidden, 16, 3),
           nn.Flatten(),
           nn.Linear(2656,31)#12544
        )

    def forward(self, x):
        """
        Shape:
            - x: (N, C, H, W)
            - output: (W, N, C)
        """

        conv = self.features(x)
        conv = self.dropout(conv)
        conv = self.last_conv_1x1(conv)

        # font = self.flatten(conv)
        # font = self.font_clasifier(font)
        font = self.classifier(conv)
        # print(conv.shape)
        # font = self.tmp1(conv)
        # print(font.shape)
        # font = self.tmp2(font)
        # print(font.shape)
        # font = self.tmp3(font)
        # print(font.shape)
        
        conv = conv.transpose(-1, -2)
        conv = conv.flatten(2)
        conv = conv.permute(-1, 0, 1)

        return conv, font


def vgg11_bn(ss, ks, hidden, pretrained=True, dropout=0.5):
    return Vgg('vgg11_bn', ss, ks, hidden, pretrained, dropout)


def vgg19_bn(ss, ks, hidden, pretrained=True, dropout=0.5):
    return Vgg('vgg19_bn', ss, ks, hidden, pretrained, dropout)

