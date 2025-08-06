const queryString = window.location.search; 
// Create a URLSearchParams object to easily access query parameters
const urlParams = new URLSearchParams(queryString);
// Get individual query parameters by name
const code = urlParams.get('code');  // 'abc'

// const formInfoDiv = document.getElementById('form-info');
// const formId = formInfoDiv.getAttribute('data-form-id');
// const formName = formInfoDiv.getAttribute('data-form-name');
const deleteTrainsButton = document.getElementById('deleteTrainBtn');
const stopTrainsButton = document.getElementById('stopTrainBtn');
const refreshTrainsButton = document.getElementById('refreshTrainBtn');
let selectedTrains = new Set();



let gpuInfos = [];
let trains = [];
fetchGpus()
fetchTrain()

function fetchGpus(){
    fetch(`/train/get-gpus`)//?form_id=${formId}&page=${page}
        .then(response => response.json())
        .then(data => {        
            gpuInfos = data.gpu_info;
            displayGpuInfo();

        })
        .catch(error => {
            console.error('Error fetching images:', error);
        });
}

function fetchTrain(){
    fetch(`/train/get-trains?code=${code}`)
        .then(response => response.json())
        .then(data => {
            // console.log(data)
            trains = data.trains; // Update the labels array with the data from the serverc
            // console.log(labels)
            displayTrainSessions(); // Update the label list in the UI
        })
        .catch(error => {
            console.error('Error fetching labels:', error);
        });
}

refreshTrainsButton.addEventListener('click', () => {
    fetchTrain()
});


stopTrainsButton.addEventListener('click', () => {
    let  selectedTrainIds = Array.from(selectedTrains)
    fetch('/train/stop-trains', {
        method: 'POST',  // Phương thức HTTP là POST
        headers: {
          'Content-Type': 'application/json',
          'X-CSRFToken': getCSRFToken(),  // Đảm bảo rằng bạn đang gửi dữ liệu ở định dạng JSON
        },
        body: JSON.stringify({train_ids: selectedTrainIds})  // Dữ liệu cần gửi, phải chuyển đổi thành JSON
        })
        .then(response => {
            if (!response.ok) {
                alert('Failed to stop selected trains.');
            }
            fetchTrain()
        })
        
        selectedTrains.clear();
});

deleteTrainsButton.addEventListener('click', () => {
    let  selectedTrainIds = Array.from(selectedTrains)
    fetch('/train/delete-trains', {
        method: 'DELETE',  // Phương thức HTTP là POST
        headers: {
          'Content-Type': 'application/json',
          'X-CSRFToken': getCSRFToken(),  // Đảm bảo rằng bạn đang gửi dữ liệu ở định dạng JSON
        },
        body: JSON.stringify({train_ids: selectedTrainIds})  // Dữ liệu cần gửi, phải chuyển đổi thành JSON
        })
        .then(response => {
            if (!response.ok) {
                alert('Failed to delete selected trains.');
            }
            fetchTrain()
        })
        
        selectedTrains.clear();
});

function displayGpuInfo() {
    const gpuContainer = document.querySelector(".gpu-info");
    gpuContainer.innerHTML = ""; // Clear any existing content

    // Tạo các thẻ GPU từ dữ liệu GPU
    gpuInfos.forEach((gpu) => {
        const gpuDiv = document.createElement("div");
        gpuDiv.classList.add("gpu-container");

        gpuDiv.innerHTML = `
        <h3>${gpu.name}</h3>
        <ul>
            <li><strong>Total GPU RAM:</strong> ${gpu.memoryTotal} MB</li>
            <li><strong>Used GPU RAM:</strong> ${gpu.memoryUsed} MB</li>
            <li><strong>GPU Temperature:</strong> ${gpu.temperature} *C</li>
            <li><strong>GPU Driver:</strong> ${gpu.driver}</li>
        </ul>
        `;
        gpuContainer.appendChild(gpuDiv);
    });
}

// function displayTrainSessions() {
//     const trainTable = document.getElementById("trainListTable").getElementsByTagName("tbody")[0];
//     trainTable.innerHTML = "";  // Clear the table body before rendering new rows

//     trains.forEach((session) => {
//         const row = trainTable.insertRow();
//         row.innerHTML = `
//         <td><input type="checkbox" data-id="${session.id}"></td>
//         <td>${session.session_name}</td>
//         <td>${session.status}</td>
//         <td>${session.progress}%</td>
//         `;
//     });
// }

function displayTrainSessions() {
    const trainTable = document.getElementById("trainListTable").getElementsByTagName("tbody")[0];
    trainTable.innerHTML = "";  // Clear the table body before rendering new rows

    trains.forEach((session) => {
        const row = trainTable.insertRow();

        // Tạo checkbox
        const checkbox = document.createElement('input');
        checkbox.type = 'checkbox';
        checkbox.setAttribute('data-id', session.id);

        // Gán sự kiện change cho checkbox
        checkbox.addEventListener('change', (event) => {
            toggleLabelSelection(event.target, session.id);
        });

        // Thêm checkbox vào cột đầu tiên
        const cell = row.insertCell(0);
        cell.appendChild(checkbox);

        // Thêm các cột khác
        row.insertCell(1).textContent = session.session_name;
        row.insertCell(2).textContent = session.status;
        row.insertCell(3).textContent = `${session.progress}%`;
        // console.log(session.accuracy)
        let acc = parseFloat(session.accuracy)*100
        row.insertCell(4).textContent = `${acc.toFixed(2)}%`;
    });
}

// Hàm toggleLabelSelection
function toggleLabelSelection(checkbox,session_id) {
    if (checkbox.checked) {
        selectedTrains.add(session_id);
    } else {
        selectedLabels.delete(session_id);
    }
    
}

window.onload = function() {
    displayGpuInfo();
    displayTrainSessions();
};

// Create Train Button
document.getElementById('createTrainBtn').addEventListener('click', function() {
    document.getElementById('popup').style.display = 'flex';
});

document.getElementById('confirmCreateTrain').addEventListener('click', function() {
    // alert('Train session created!');
    document.getElementById('popup').style.display = 'none';
    trainModel();
    fetchTrain();
    displayTrainSessions();  // Cập nhật lại danh sách train
});

// Cancel Create Train in Popup
document.getElementById('cancelCreateTrain').addEventListener('click', function() {
    document.getElementById('popup').style.display = 'none';
});


function trainModel() {
    // const trainButton = document.getElementById('trainButton');
    // const stopButton = document.getElementById('stopButton');

    // Gửi yêu cầu đến backend để bắt đầu huấn luyện
    fetch('/train/train-model', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
            // 'X-CSRFToken': getCSRFToken(),
        },
        body: JSON.stringify({ code: code })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            alert('Mô hình đã được đưa vào huấn luyện!');
            
        } else {
            alert('Chưa được đưa vào hàng chờ');
        }
    })
    .catch(error => {
        statusDiv.innerHTML = 'Lỗi kết nối với server!';
        console.error('Error:', error);
    });

    
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