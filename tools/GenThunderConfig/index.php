<?php
header("Content-type:text/html;charset=utf-8");
include("./func.php");

if (!isset($_GET['df'])) {
	/**
	 * 列表目录
	 */
	
	$files = dirTree(dirname(__FILE__) . '/iso/');
	if (sizeof($files) < 1) {
		die("<h2>目录下没有可供使用的文件!</h2>");
	} else {
		if(isset($_GET['fmt']) &&  strtolower($_GET['fmt']) == 'html'){
			echo '<ol>';
			for($i = 0; $files[$i] != null; $i++) {
				echo '<li><a href="/?df=' . mb_convert_encoding($files[$i], "utf-8", "gb2312") . '" target="_blank">' . mb_convert_encoding($files[$i], "utf-8", "gb2312") . '</a></li>';
			} 
			echo '</ol>';
		}else{
			$farr = array();
			foreach($files as $f){
				$farr[] = 'http://'.$_SERVER['SERVER_NAME'] . ':' . $_SERVER['SERVER_PORT'] . $_SERVER['SCRIPT_NAME'].'?df='.mb_convert_encoding($f, "utf-8", "gb2312");
			}
			die(json_encode($farr));
		}
	} 
} else {
	/**
	 * 文件下载
	 */
	$file_name = $_GET['df']; 
	// 用以解决中文不能显示出来的问题
	$file_name = charWeb2Console($file_name);
	$file_sub_path = $_SERVER['DOCUMENT_ROOT'] . "/iso/";
	$file_path = $file_sub_path . $file_name; 
	// 首先要判断给定的文件存在与否
	if (!file_exists($file_path)) {
		die('文件: [' . $file_name . '] 没有找到!');
	} 
	
	try{
	$fp = fopen($file_path, "rb");
	clearstatcache();
	//$file_size = filesize($file_path); 
	$file_size = sprintf("%u", filesize($file_path));
	//var_dump($file_size);
	//exit;
	// 下载文件需要用到的头
	Header("Content-type: application/octet-stream");
	Header("Accept-Ranges: bytes");
	Header("Accept-Length:" . $file_size);
	Header("Content-Length:" . $file_size);
	Header("Content-Disposition: attachment; filename=" . $file_name);
	$buffer = 1024;
	$file_count = 0; 
	// 向浏览器返回数据
	// while(!feof($fp) && $file_count<$file_size-$buffer){
	while ($file_count < $file_size - $buffer) {
		$file_con = fread($fp, $buffer);
		$file_count += $buffer;
		echo $file_con;
	} 
	fclose($fp);
	}catch(Exception $e){
		print_r($e);
	}
}