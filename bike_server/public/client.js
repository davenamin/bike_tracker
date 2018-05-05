// client-side js

$(function() {
  console.log('hello world :o');
  var map = L.map('map');
  //L.esri.Vector.basemap("Navigation").addTo(map);
  L.esri.basemapLayer("Gray").addTo(map);
  var data = $.get('/data', function(data) {
    var gpx = L.geoJSON(data, {
    style: function (feature) {
        return {color: feature.properties.color || "red"};
    }
    }).bindPopup(function (layer) {
    return layer.feature.properties.time;
    }).addTo(map);
    map.fitBounds(gpx.getBounds());
  });
});
