<?php
//遍历目录
function dirTree($dirName=null) {
    if(empty($dirName))exit("Directory Is Empty.");
    if(is_dir($dirName)) {
        if($dh=opendir($dirName)) {
            $tree=array();
            while(($file=readdir($dh))!==false) {
                if($file!="."&&$file!="..") {
                    $filePath=$dirName."/".$file;
                    if(is_dir($filePath)) {
                        $tree[$file]=dirTree($filePath);
                    } else {
                        $tree[]=$file;
                    }
                }
            }
            closedir($dh);
        } else {
            exit("Can't Open Directory $dirName.");
        }
        return$tree;
    } else {
        exit("$dirName Is Not A Directory.");
    }
}

function charWeb2Console($str){
	if("WINNT" == PHP_OS || "WINDOWS" == PHP_OS){
		//system("CHCP 65001");
		$detectCharSet = 'UTF-8';
		$ConsoleCharSet = "GB2312";
		$destStr = mb_convert_encoding($str, $ConsoleCharSet, $detectCharSet);
		return $destStr;
	}
	return $str;
}
