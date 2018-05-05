// client-side js

$(function () {
  console.log('hello world :o');
  var map = L.map('map');
  //L.esri.Vector.basemap("Navigation").addTo(map);
  L.esri.basemapLayer("Gray").addTo(map);
  var routeCols = palette("rainbow", 365);
  var data = $.get('/data', function (datalist) {
    datalist.forEach(function (data) {

      var gpx = L.geoJSON(data, {
        style: function (feature) {
          // get the color by day of year, inspired from:
          // https://stackoverflow.com/a/8619946
          var date = new Date(feature.properties.time);
          var yearstart = new Date(date.getFullYear(), 0, 0);
          var diff = date - yearstart;
          var oneDay = 1000 * 60 * 60 * 24;
          var day = Math.floor(diff / oneDay);
          return {
            color: "#" + routeCols[day]
          };
        }
      }).bindPopup(function (layer) {
        return layer.feature.properties.time;
      }).addTo(map);
      map.fitBounds(gpx.getBounds());
    });
  });
});