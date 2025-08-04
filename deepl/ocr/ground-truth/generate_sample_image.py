from PIL import Image, ImageDraw, ImageFont

# Tạo ảnh trắng
img = Image.new('RGB', (600, 200), color = (255,255,255))
d = ImageDraw.Draw(img)

# Nội dung
text = "Ho va ten: Nguyen Van A\nngay sinh: 01/01/1990\nque quan: Ha Noi\ncho o hien tai: TP Ho Chi Minh"

# Chọn font (dùng font mặc định, nếu muốn font viết tay cần tải thêm)
d.text((10,10), text, fill=(0,0,0))

img.save('sample1.png')
print('Đã tạo ảnh mẫu: sample1.png')
