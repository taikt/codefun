const queryString = window.location.search; 
const HOST = "http://10.0.20.212:9322/"; 
const urlParams = new URLSearchParams(queryString);
const code = urlParams.get('code'); 
const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d');

// Hàm để tính toán vị trí ảnh trong canvas hiện tại
const ocanvas = document.getElementById('overlaycanvas');
const octx = ocanvas.getContext('2d');


const labelList = document.getElementById('labels');
const imageList = document.getElementById('images');

// let currentPage = 1;
let currentImageIndex = 0;  // Chỉ số ảnh hiện tại

let selectedLabels = new Set();  // Set to store selected labels
let selectedImages = new Set();
let labels = [];
let images = []; 

let selectedLabelColor = null;
let selectedLabelId = null;
let selectedImageId = null;
let boundingBoxes = []; // Danh sách các bounding box đã vẽ
let selectedImageWidth = null
let selectedImageHeight = null
let selectedImageX = null
let selectedImageY = null


fetchLabels();
fetchImages();

// Nút Next và Previous
document.getElementById('next-image').addEventListener('click', () => {
    saveLabel(selectedImageId);
    if (currentImageIndex < images.length - 1) {
        currentImageIndex++;
    }
    loadImageToCanvasByIndex(currentImageIndex);
});

document.getElementById('prev-image').addEventListener('click', () => {
    saveLabel(selectedImageId);
    if (currentImageIndex > 0) {
        currentImageIndex--;
    }
    loadImageToCanvasByIndex(currentImageIndex);
});


// Get label
function fetchLabels() {
    fetch(HOST+`CoreOCR/GetAllFieldByTemplateOcr?code=${code}`)
        .then(response => response.json())
        .then(data => {
            // console.log(data.data)
            labels = data.data; // Update the labels array with the data from the serverc
            // console.log(labels)
            updateLabelList(); // Update the label list in the UI
        })
        .catch(error => {
            console.error('Error fetching labels:', error);
        });
}

function fetchImages() {
    fetch(HOST+`CoreOCR/GetAllFileByTemplateOcr?code=${code}`)//?form_id=${formId}&page=${page}
        .then(response => response.json())
        .then(data => {
            
            images = data.data;
            console.log(images)
            updateImageList()
            
        })
        .catch(error => {
            console.error('Error fetching images:', error);
        });
}

function updateLabelList() {
    labelList.innerHTML = ''; // Clear the current label list
    labels.forEach(label => addLabelToList(label.fieldName, label.code, label.colorField)); // Add labels from the server
}

function updateImageList() {
    imageList.innerHTML = ''; // Clear the current label list
    images.forEach((image,index) => addImageToList(image, index)); // Add labels from the server
}

// Add new label to the label list
function addLabelToList(labelName, id ,color) {
    const li = document.createElement('li');
    li.classList.add('list-group-item');
    li.style.position = 'relative';
    
    const labelText = document.createElement('span');
    labelText.classList.add('label-text');
    labelText.textContent = labelName;
    
    const labelOverlay = document.createElement('div');
    labelOverlay.classList.add('label-overlay');
    labelOverlay.style.backgroundColor = color;

    // Create checkbox for selection
    const checkboxContainer = document.createElement('div');
    checkboxContainer.classList.add('checkbox-container');
    const checkbox = document.createElement('input');
    checkbox.type = 'checkbox';
    checkbox.classList.add('label-checkbox');
    checkbox.style.marginRight = '10px';
    checkbox.value = id; // thêm id vào checkbox
    checkbox.addEventListener('change', (event) => {
        toggleLabelSelection(event.target, labelName, id, color);
    });
    // checkbox.onclick = () => selectLabel(id, color);

    checkboxContainer.appendChild(checkbox);
    li.appendChild(checkboxContainer);
    li.appendChild(labelText);
    li.appendChild(labelOverlay);

    labelList.appendChild(li);
}

function addImageToList(image, index = 0) {
    console.log(image.url)
    const li = document.createElement('li');
    li.classList.add('list-group-item');
    li.style.position = 'relative';
    
    const link = document.createElement('a');
    link.href = '#';
    link.textContent = image.fileName;
    link.dataset.imageUrl = image.url; // Thêm URL ảnh vào dữ liệu của liên kết
    link.dataset.index = index;  // Thêm chỉ số index vào dữ liệu của liên kết
    link.dataset.img_id = image.id;
    link.addEventListener('click', (e) => loadImageToCanvasByIndex(index));

    // Tạo checkbox và thêm vào thẻ link
    const checkboxContainer = document.createElement('div');
    checkboxContainer.classList.add('checkbox-container');
    
    const checkbox = document.createElement('input');
    checkbox.type = 'checkbox';
    checkbox.classList.add('image-checkbox');
    checkbox.value = image.id;  // Lưu ID ảnh vào checkbox để xóa ảnh

    // Khi checkbox thay đổi, gọi hàm để xử lý xóa ảnh
    checkbox.addEventListener('change', (event) => {
        handleImageSelection(event.target, image.id);
    });

    checkboxContainer.appendChild(checkbox);
    link.appendChild(checkboxContainer);

    li.appendChild(link);
    imageList.appendChild(li);
}

// Toggle label selection state
function toggleLabelSelection(checkbox, labelName, id, color) {
    if (checkbox.checked) {
        selectedLabels.add(labelName);
        selectedLabelId = id;
        selectedLabelColor = color;
    } else {
        selectedLabels.delete(labelName);
        selectedLabelId = null;
        selectedLabelColor = null;
    }
    
}

function getCSRFToken() {
    let csrfToken = null;
    if (document.cookie) {
        document.cookie.split(';').forEach(cookie => {
            const [name, value] = cookie.trim().split('=');
            if (name === 'csrftoken') {
                csrfToken = value;
            }
        });
    }
    return csrfToken;
}


function handleImageSelection(checkbox, imageId) {
    if (checkbox.checked) {
        selectedImages.add(imageId);
    } else {
        selectedImages.delete(imageId);
    }

    // Cập nhật trạng thái nút xóa ảnh
    updateDeleteButtonState();
}


function loadImageToCanvasByIndex(index){
    clearLabel()
    currentImageIndex = index
    const image = images[index];
    
    const img = new Image();
    let imgWidth, imgHeight;  // Kích thước thực tế của ảnh
    let scale;       // Tỷ lệ scale giữa ảnh và canvas
    // console.log(image.image_url)
    img.src = image.url;
    selectedImageId = image.id
    // console.log(selectedImageId)
    img.onload = () => {
        imgWidth = img.width;
        imgHeight = img.height;

        // Cập nhật kích thước của canvas và overlaycanvas dựa trên ảnh
        const canvasWidth = ocanvas.clientWidth;
        const canvasHeight = ocanvas.clientHeight;

        if (canvasWidth/canvasHeight > imgWidth/imgHeight){
            scale = canvasHeight/imgHeight;
        }
        else{
            scale = canvasWidth/imgWidth;
        } 

        // Cập nhật kích thước của canvas để phù hợp với tỷ lệ của ảnh
        ocanvas.width = canvasWidth//imgWidth * scale;
        ocanvas.height = canvasHeight//imgHeight * scale;

        // Vẽ ảnh lên canvas chính
        const canvas = document.getElementById('canvas');
        const ctx = canvas.getContext('2d');
        canvas.width = canvasWidth//imgWidth * scale;
        canvas.height = canvasHeight//imgHeight * scale;

        if (canvasWidth/canvasHeight > imgWidth/imgHeight){
            
            selectedImageX = (canvasWidth-imgWidth*scale)/2
            selectedImageY = 0
            // scale = canvasHeight/imgHeight;
        }
        else{
            // scale = canvasWidth/imgWidth;
            selectedImageX = 0
            selectedImageY = (canvasHeight-imgHeight*scale)/2
        } 
        selectedImageHeight = imgHeight * scale
        selectedImageWidth = imgWidth * scale
        // console.log(selectedImageWidth, selectedImageHeight, selectedImageX, selectedImageY)
        ctx.drawImage(img, selectedImageX, selectedImageY, selectedImageWidth, selectedImageHeight);

        
        (async () => {
            img_labels = await getLabel(image.id)
            console.log('Label data:', img_labels.label);
            if (img_labels!={})
            {
                boundingBoxes = convertLabelToCanvas(img_labels.label, selectedImageWidth, selectedImageHeight, selectedImageX, selectedImageY)
            }
            redraw();
        })();
    }
}


function saveLabel(imageId){
    // upload label to database
    convertedBoxes = convertCanvastoLabel(boundingBoxes, selectedImageWidth, selectedImageHeight, selectedImageX, selectedImageY);
    fetch(HOST+"CoreOCR/UpdateAreaFile", {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
            // 'X-CSRFToken': getCSRFToken(),
        },
        body: JSON.stringify({id: imageId,area: JSON.stringify({ label: convertedBoxes })}),
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            return;
        } else {
            // alert('Error creating label: ' + data.error);
            return;
        }
    })
    .catch(error => {
        console.error('Error creating label:', error);
    });


}


// async function getLabel(imageId) {
//     try {
//         const response = await fetch(`CoreOCR/GetFileById?id=${imageId}`);
//         const data = await response.json();
        
//         let lb = data.data.area;
//         return lb;
//     } catch (error) {
//         console.error('Error fetching images:', error);
//         return {};
//     }
// }
async function getLabel(imageId) {
    try {
        const response = await fetch(HOST+`CoreOCR/GetFileById?id=${imageId}`);
        
        // Kiểm tra xem response có thành công không
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        const data = await response.json();
        
        // Kiểm tra cấu trúc dữ liệu trả về
        if (data && data.data && data.data.area) {
            return JSON.parse(data.data.area);
        } else {
            console.warn('Unexpected API response structure:', data);
            return null; // Hoặc giá trị mặc định nếu cần
        }
    } catch (error) {
        console.error('Error fetching labels:', error);
        return null; // Hoặc giá trị mặc định nếu cần
    }
}


let isDrawing = false;
let isResizing = false;
let isDragging = false;
let dragOffsetX = 0;
let dragOffsetY = 0;
let currentBox = null;
let resizeCorner = null;
let selectedBox = null; // Box currently selected for overlay

const RESIZE_THRESHOLD = 10;
const HANDLE_RADIUS = 5;

// Event listeners for ocanvas
ocanvas.addEventListener('mousedown', startDrawing);
ocanvas.addEventListener('mousemove', drawOrMove);
ocanvas.addEventListener('mouseup', stopDrawingOrMoving);

function startDrawing(e) {
    const { x, y } = getMousePos(ocanvas, e);

    for (let box of boundingBoxes) {
            
        const resizeCornerInfo = getResizeCorner(x, y, box);
        // console.log(resizeCornerInfo)
        if (resizeCornerInfo) {
            // Nếu click vào góc của bounding box, vào chế độ resize
            isResizing = true;
            currentBox = box;
            resizeCorner = resizeCornerInfo;
            clickedOnBox = true; // Đánh dấu là đã click vào một box
            return;
        }
        if (isInsideBox(x, y, box)) {
            // Nếu click vào vùng bên trong của bounding box, chọn box đó
            selectedBox = box;
            currentBox = box;
            clickedOnBox = true;
            isDragging = true;
            dragOffsetX = x - box.x;
            dragOffsetY = y - box.y;
            return;
        }
    }
    isDrawing = true;
    currentBox = {x, y, width: 0, height: 0, label_id: selectedLabelId, color: selectedLabelColor};
    // console.log(currentBox)
    boundingBoxes.push(currentBox);  // Add new box to boundingBoxes
}

function drawOrMove(e) {
    const { x, y } = getMousePos(ocanvas, e);

    if (isDrawing) {
        currentBox.width = x - currentBox.x;
        currentBox.height = y - currentBox.y;
        if (x < currentBox.x){
            // currentBox.width = currentBox.x - x;
            currentBox.rx = true
            // currentBox.x = x;
        }
        if (y < currentBox.y){
            // currentBox.height = currentBox.y - y;
            currentBox.ry = true
            // currentBox.x = x;
        }
        redraw();
    } else if (isDragging) {
        currentBox.x = x - dragOffsetX;
        currentBox.y = y - dragOffsetY;
        redraw();
    } else if (isResizing) {
        resizeBox(currentBox, x, y, resizeCorner);
        redraw();
    }
}

function stopDrawingOrMoving() {
    isDrawing = false;
    isDragging = false;
    isResizing = false;
    if (currentBox.width<0)
    {
        currentBox.x = currentBox.x+currentBox.width;
        currentBox.width = - currentBox.width;
    }
    if (currentBox.height<0)
        {
            currentBox.y = currentBox.y+currentBox.height;
            currentBox.height = - currentBox.height;
        }
    currentBox = null;
    resizeCorner = null;
}

function resizeBox(box, mouseX, mouseY, corner) {
    switch (corner) {
        case 'tl':
            box.width += box.x - mouseX;
            box.height += box.y - mouseY;
            box.x = mouseX;
            box.y = mouseY;
            break;
        case 'tr':
            box.width = mouseX - box.x;
            box.height += box.y - mouseY;
            box.y = mouseY;
            break;
        case 'bl':
            box.width += box.x - mouseX;
            box.x = mouseX;
            box.height = mouseY - box.y;
            break;
        case 'br':
            box.width = mouseX - box.x;
            box.height = mouseY - box.y;
            break;
    }
}

function redraw() {
    octx.clearRect(0, 0, ocanvas.width, ocanvas.height);

    for (let box of boundingBoxes) {
        // console.log(box)
        // Set the color based on the class of the bounding box
        octx.strokeStyle = box.color;//selectedLabelColor;
        octx.lineWidth = 2;

        // Draw the overlay if this box is selected
        if (box === selectedBox) {
            octx.fillStyle = selectedLabelColor;
            octx.globalAlpha = 0.2; // Set opacity for overlay
            octx.fillRect(box.x, box.y, box.width, box.height); // Overlay
            octx.globalAlpha = 1; // Reset opacity to full for the outline
        }

        octx.strokeRect(box.x, box.y, box.width, box.height);
        drawHandles(box);
    }
}

function drawHandles(box) {
    const corners = [
        { x: box.x, y: box.y }, // top-left
        { x: box.x + box.width, y: box.y }, // top-right
        { x: box.x, y: box.y + box.height }, // bottom-left
        { x: box.x + box.width, y: box.y + box.height } // bottom-right
    ];

    octx.fillStyle = 'green';
    for (const corner of corners) {
        octx.beginPath();
        octx.arc(corner.x, corner.y, HANDLE_RADIUS, 0, 2 * Math.PI);
        octx.fill();
    }
}

function isInsideBox(x, y, box) {
    return x > box.x && x < box.x + box.width && y > box.y && y < box.y + box.height;
}

function getResizeCorner(x, y, box) {
    const corners = [
        { name: 'tl', x: box.x, y: box.y }, // top-left
        { name: 'tr', x: box.x + box.width, y: box.y }, // top-right
        { name: 'bl', x: box.x, y: box.y + box.height }, // bottom-left
        { name: 'br', x: box.x + box.width, y: box.y + box.height } // bottom-right
    ];

    for (const corner of corners) {
        if (Math.abs(x - corner.x) < RESIZE_THRESHOLD && Math.abs(y - corner.y) < RESIZE_THRESHOLD) {
            return corner.name;
        }
    }
    return null;
}

function getMousePos(ocanvas, evt) {
    const rect = ocanvas.getBoundingClientRect();
    return {
        x: evt.clientX - rect.left,
        y: evt.clientY - rect.top
    };
}

function convertCanvastoLabel(boxes, imageWidth, imageHeight, imageX, imageY){
    
    const convertedBoxes = [];

    for (let i = 0; i < boxes.length; i++) {
        const box = boxes[i];
        const relativeX = (box.x - imageX) / imageWidth; // Normalize x relative to image width
        const relativeY = (box.y - imageY) / imageHeight; // Normalize y relative to image height
        const relativeWidth = box.width / imageWidth;    // Normalize width relative to image width
        const relativeHeight = box.height / imageHeight; // Normalize height relative to image height

        convertedBoxes.push({
            x: relativeX,
            y: relativeY,
            width: relativeWidth,
            height: relativeHeight,
            label_id : box.label_id,
        });
    }

    return convertedBoxes;
    
}

function convertLabelToCanvas(boxes, imageWidth, imageHeight, imageX, imageY){
    const absoluteBoxes = [];

    for (let i = 0; i < boxes.length; i++) {
        const box = boxes[i];
        const absoluteX = (box.x * imageWidth) + imageX; // Convert x back to absolute value
        const absoluteY = (box.y * imageHeight) + imageY; // Convert y back to absolute value
        const absoluteWidth = box.width * imageWidth;    // Convert width back to absolute value
        const absoluteHeight = box.height * imageHeight; // Convert height back to absolute value
        let color = "#000000";
        for ( let j = 0; j < labels.length; j++)
        {   
            if (labels[j].id==box.label_id)
            {
                color = labels[j].color;
                break;
            }
        }
        absoluteBoxes.push({
            x: absoluteX,
            y: absoluteY,
            width: absoluteWidth,
            height: absoluteHeight,
            label_id : box.label_id,
            color: color
        });
    }

    return absoluteBoxes;
}
function clearLabel(){
    boundingBoxes = []
}

document.addEventListener('keydown', function(event) {
    if (event.key === 'Delete') {
        if (selectedBox) {
            boundingBoxes = boundingBoxes.filter(box => box !== selectedBox); // Remove the selected box from the array
            // console.log(boundingBoxes)
            selectedBox = null; // Deselect the box
            redraw(); // Redraw the ocanvas
        }
    }
  });

