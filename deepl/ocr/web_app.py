from flask import Flask, request, render_template_string
from preprocess import preprocess_image
from extract_fields import extract_fields

app = Flask(__name__)

HTML_FORM = '''
<!doctype html>
<title>OCR Tờ khai cư trú</title>
<h2>Upload ảnh tờ khai cư trú</h2>
<form method=post enctype=multipart/form-data>
  <input type=file name=file>
  <input type=submit value=Upload>
</form>
{% if fields %}
<h3>Kết quả trích xuất:</h3>
<ul>
  <li>Họ tên: {{ fields['ho_ten'] }}</li>
  <li>Năm sinh: {{ fields['nam_sinh'] }}</li>
  <li>Quê quán: {{ fields['que_quan'] }}</li>
  <li>Nơi ở hiện tại: {{ fields['noi_o'] }}</li>
</ul>
{% endif %}
'''

@app.route('/', methods=['GET', 'POST'])
def upload_file():
    fields = None
    if request.method == 'POST':
        file = request.files['file']
        if file:
            filepath = f"/tmp/{file.filename}"
            file.save(filepath)
            processed_path = preprocess_image(filepath)
            fields = extract_fields(processed_path)
    return render_template_string(HTML_FORM, fields=fields)

if __name__ == "__main__":
    app.run(debug=True)
