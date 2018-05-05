/* jshint esversion: 6 */
// server.js
// where your node app starts

// init project
const express = require('express');
const app = express();

const togeojson = require('togeojson');
const fs = require('fs');
const jsdom = require("jsdom");
const {
  JSDOM
} = jsdom;

// use pug
app.set('view engine', 'pug');

// static file server
app.use(express.static('public'));

app.get('/', function (req, res) {
  res.render('index', {});
});

// Simple in-memory store
var data = [];

fs.readdir('examples', (err, files) => {
  files.forEach(function (file) {
    var gpx = new JSDOM(fs.readFileSync('examples/' + file, 'utf8')).window.document;
    data.push(togeojson.gpx(gpx));
  });
});

app.get("/data", (request, response) => {
  response.send(data);
});

// listen for requests :)
const listener = app.listen(process.env.PORT, () => {
  console.log(`Your app is listening on port ${listener.address().port}`);
});