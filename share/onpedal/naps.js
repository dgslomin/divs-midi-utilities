
var Shell = WScript.CreateObject("WScript.Shell");

Shell.AppActivate("Not Another PDF Scanner 2");
Shell.SendKeys("{F2}");

while (!Shell.AppActivate("EPSON Scan")) {
	WScript.Sleep(250);
}

Shell.SendKeys("%s");

