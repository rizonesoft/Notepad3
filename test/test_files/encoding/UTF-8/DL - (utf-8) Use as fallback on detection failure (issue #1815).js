javascript:!function(){
  var userName = location.href.split("/")[4];
  var fileName = "Twilog-" + userName + ".html";
  var fileNameForAll = "Twi " + userName + ".html";

  $(document).ready(function(){
    $("#loading").before('<a id="save" href="javascript:void(0);" title="adding">📃</a><a id="saveall" href="javascript:void(0);" style="margin-left:20px;">All(First)</a><input id="nest" style="margin-left:20px;" type="checkbox"><input type="text" id="newest" size="10">ネスト');
    $('#save').css("size", "50px").click(function() { idSave(fileName) });
    $('#saveall').css("size", "50px").click(function() { idSaveAll(fileNameForAll, userName) });
  });

  function idSaveAll(fName, id) {
    var text = conv( $("#results").html(), true );
    if (text =="") return;
    
    var nest = "", LocalSaveFolder = "save", SaveFolderName = LocalSaveFolder;
    if ($('#nest').is(':checked'))
      var nest = "../../../", SaveFolderName = "FX" + "/" + LocalSaveFolder;
    var header = ['<!DOCTYPE html>', '<html>', '<meta charset="UTF-8">', '<title>old tweets:</title><base target="_blank" />',
      '<link rel="stylesheet" href="' + nest + SaveFolderName + '/default.css" />',
      '<script src="' + LocalSaveFolder + '/Twi ' + id + '.txt"></script>',
      '<script src="' + nest + SaveFolderName + '/jquery.min.js"></script>',
      '<script src="' + nest + SaveFolderName + '/default.js"></script>',
      '</head><body>',
      '<div id="nav"></div>',
      '<div id="results">'].join("\r\n");
    var footer = '</div></body></html>';
   // alert(fName + "\n" + text.slice(0,200))
    
    saveFile(fName, [header, text, footer].join("\r\n"));
  }
  function idSave(fName) {
    var text = conv( $("#results").html() );
    if (text =="") return;
    saveFile(fName, text);
  }
  function saveFile(fName, text) {
    var blob = new Blob([text], {type: "text/plain;charset=UTF-8"});
    if (window.navigator.msSaveBlob)
      window.navigator.msSaveBlob(blob, fName); // IE
    else { // それ以外
     var a = document.createElement("a");
      a.href = URL.createObjectURL(blob);
      a.download = fName;
      a.click();
    }  
  }
  
  function conv(s, isAll) {
    if (isAll != false && $("#newest").val() != "") {
      var targetId = $("#newest").val().split(":")[0];
      if (location.href.indexOf(targetId) == -1) {
        alert("ユーザー名が一致しません。");
        $('input[name="new_user_box"]').val(targetId);
        return "";
        }
      var newest = $("#newest").val().split(":")[1];
      
      var t = s.split(newest);
      if (t[1] == undefined) {
        alert("status id が見つかりません。Load 数が増えるのを待ってみてください。");
        return "";
      }
      var ind = t[0].lastIndexOf('</span>');
      s = t[0].substring(0, ind) + '</span>';
    } else {
      if (isAll == false) {
        alert("status id を入力してください。");
        return "";
      }
    }
    
    
    s = s.split("/span><br><span").join("/span><span");
    var sp1 = "<span ", kugiri = "class=\"tweet\">", ume = sp1+kugiri, kara = "";
    var b = s.split("<span rel=\"");
    var a = [];
    for (var i in b) {
      if (i == 0) a.push(b[i]);
      else {
        a.push(ume);
        var t = b[i].split(kugiri)[1];
        a.push(t);
      }
    }
    a[a.length-1] = a[a.length-1].replace("</span><br>", "</span>");
    s = a.join(kara);
    if (10 > s.length) { alert("更新がないようです。"); return ""; }
    return s;
  }

}();void 0;