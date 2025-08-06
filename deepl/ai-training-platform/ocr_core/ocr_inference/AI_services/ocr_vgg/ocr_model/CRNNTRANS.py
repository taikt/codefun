
from torch import nn
from .vgg import Vgg
from .transformer  import LanguageTransformer
class OCR(nn.Module):
    def __init__(self, vocab_size):

        super(OCR, self).__init__()

        ss = [[2, 2], [2, 2], [2, 1], [2, 1], [1, 1]] # reduce dimention of vgg output [[2, 2], [2, 2], [2, 2], [2, 1], [1, 1]]
        ks = [[2, 2], [2, 2], [2, 1], [2, 1], [1, 1]]

        hidden = 256

        pretrained = False

        dropout = 0.5

        self.cnn = Vgg('vgg19_bn', ss, ks, hidden, pretrained, dropout)

        self.transformer = LanguageTransformer(vocab_size, d_model= 256,
                 nhead = 8,
                 num_encoder_layers= 6,
                 num_decoder_layers= 6 ,
                 dim_feedforward= 2048,
                 max_seq_length= 1024,
                 pos_dropout= 0.1,
                 trans_dropout= 0.1)



    def forward(self, img, tgt_input, tgt_key_padding_mask):
        """
        Shape:
            - img: (N, C, H, W)
            - tgt_input: (T, N)
            - tgt_key_padding_mask: (N, T)
            - output: b t v
        """
        src, font = self.cnn(img) 
        outputs,output_BUI = self.transformer(src, tgt_input, tgt_key_padding_mask=tgt_key_padding_mask)

        return outputs,output_BUI,font

