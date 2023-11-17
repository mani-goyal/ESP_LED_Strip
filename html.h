/*****************************************************************    
 *    htmlCode[]
 * 
 *    Store the webpage in Program Memory during upload
 *****************************************************************/
const char htmlCode[] PROGMEM =
R"=====(
<!DOCTYPE html>
<html>
   <head>
      <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">
      <link rel=\"icon\" href=\"data:,\">
      <link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css\">
      <script src=\"https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.0.4/jscolor.min.js\"></script>
   </head>
   <body>
      <div class=\"container\">
         <div class=\"row\">
            <h1>LED strip control</h1>
         </div>
         <a class=\"btn btn-primary btn-lg\" href=\"#\" id=\"change_color\" role=\"button\">Change Color</a> 
         <input class=\"color\" id=\"color_selector\">
         <input type="range" id="brightness-slider" "name="Brightness" min="0" max="255">
      </div>
      <script>function update(picker) {document.getElementById('rgb').innerHTML = Math.round(picker.rgb[0]) + ', ' +  Math.round(picker.rgb[1]) + ', ' + Math.round(picker.rgb[2]);
      </script>
   </body>
</html>
)=====";
