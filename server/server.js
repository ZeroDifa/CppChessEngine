const { spawn } = require('child_process');
const express = require('express');

const calculator = spawn('../main.exe');

calculator.stdout.on('data', (data) => {
    console.log(`Output: \n${data.toString().trim()}`);
});

//
calculator.stderr.on('data', (data) => {
    console.error(`Error: ${data.toString().trim()}`);
});

calculator.on('close', (code) => {
    console.log(`Calculator process exited with code ${code}`);
});

function calculate(input) {
    return new Promise((resolve, reject) => {
        console.log(input);
        calculator.stdin.write(input + '\n');
        calculator.stdout.once('data', (data) => {
            resolve(data.toString().trim());
        });
        calculator.stderr.once('data', (data) => {
            reject(data.toString().trim());
        });
    });
}


const app = express();

app.use(express.static('./client'));
app.listen(8080, () => {
    console.log('Server is running on http://localhost:8080');
    console.log('Press Ctrl+C to stop the server');
});
// express rest methods

app.get('/command', (req, res) => {
    const input = req.query.input;
    calculate(input)
       .then((result) => {
            res.send(result);
        })
       .catch((error) => {
            res.status(500).send(error);
        });
});

//stdin commander
process.stdin.on('data', (data) => {
    calculate(data.toString().trim()).then((result) => {})
})
