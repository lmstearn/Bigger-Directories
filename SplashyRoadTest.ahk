#NoEnv
#MaxMem 256
#Include %A_ScriptDir%
;EnvSet, AutGUI, Some text to put in the variable.
#include C:\Users\New\Desktop\Splashy.ahk

; Splashy requires Last Found
Gui +LastFound

SplashRef := Splashy.SplashImg

SysGet, md, MonitorWorkArea, 1

scrWd := mdRight - mdleft
scrHt := mdBottom - mdTop

;image size: 220 * 71 for wikimedia image
ahkWd := 220
ahkHt := 71
ahkHtMod := 0 ; modified

spr := 0
spr1 := 0


; Compute radius: triangle, ellipse
rWd := (scrWd - ahkWd)/2 ; allow for img width
rHt := (scrHt - ahkHt)/2 ; and img Height



startPointWd := 0
startPointHt := 0


triXArg := []
triYArg := []
triVertLength := floor (sqrt((scrWd - ahkWd) * (scrWd - ahkWd)/4 + (scrHt - ahkHt) * (scrHt - ahkHt)))


pi := 4 * ATan(1)
s := round(2 * scrHt/100)
; s doesn't want to be less than 10 or it falls victim to Splaqshy's proportion rule.

; s must be even for the ellipse
	if (mod(s, 2))
	s += 1

; arbitrary tolerance
XY_TOL := s/(scrWd + scrHt)

minIndexTop := 0
minIndexBot := 0
ellXArg := []
ellYArg := []
ellVertInterval := []



ahkWdMod := floor((s + 1) * ahkWd/s)
ahkWdAdjust := floor(mod(scrWd, ahkWdMod)/(scrWd/ahkWdMod))
ahkHtMod := floor((s + 1) * ahkHt/s)
;ahkHtAdjust := round(mod((scrHt - 2 * ((s + 1) * ahkHt/s)), ((s + 1) * ahkHt/s))/((scrHt - 2 * ((s + 1) * ahkHt/s))/((s + 1) * ahkHt/s)))
ahkHtAdjust := floor(mod((scrHt - 2 * ahkHtMod), ahkHtMod)/((scrHt - 2 * ahkHtMod)/ahkHtMod))



; init ellipse:

	Loop, %s% ; number of steps
	{
	spr := Sin(A_Index * 2 * pi/s)

	ellYArg.Push(spr)
	ellVertInterval.Push(abs(spr))
	spr1 := Cos(A_Index * 2 * pi/s)
	ellXArg.Push(spr1)
	}

spr := 1
	Loop, % s/2
	{
		if (A_Index == 1)
		ellVertInterval[A_Index] -= ellVertInterval[s]
		else
		ellVertInterval[A_Index] -= abs(ellYArg[A_Index - 1])

		if (abs(ellVertInterval[A_Index]) <= spr)
		{
		;msgbox % ellVertInterval[A_Index]
		spr := ellVertInterval[A_Index]
		minIndexBot := A_Index
		}
	}

if abs((ellVertInterval[minIndexBot] - ellVertInterval[minIndexBot - 1])) < XY_TOL
twoBots := 1
else
twoBots := 0

spr := 1
	Loop, %s%
	{
		if (A_Index > s/2)
		{
		ellVertInterval[A_Index] -= abs(ellYArg[A_Index - 1])

			if (abs(ellVertInterval[A_Index]) <= spr)
			{
			spr := ellVertInterval[A_Index]
			minIndexTop := A_Index
			}
		}
	}

	if abs((ellVertInterval[minIndexTop] - ellVertInterval[minIndexTop - 1])) < XY_TOL
	twoTops := 1
	else
	twoTops := 0




; Init image
%SplashRef%(Splashy, {vBorder: "wscd", vHide: 1, imageUrl: "https://upload.wikimedia.org/wikipedia/commons/thumb/3/36/AutoHotkey_logo.png/220px-AutoHotkey_logo.png"}*)


; ideas
;vImgTxtSize





spr := 0


Loop %s%
{
spr := (A_Index - 1)/s
%SplashRef%(Splashy, {vHide: 0, vImgW: s + spr * ahkWd, vImgH: s + spr * ahkHt}*)
sleep 5
}
msgbox
Loop %s%
{
spr := (s - A_Index)/s
%SplashRef%(Splashy, {vHide: 0, vImgW: s + spr * ahkWd, vImgH: s + spr * ahkHt}*)
sleep 5
}
msgbox
Loop
{
spr := ahkWd + s * A_Index
spr1 := ahkHt + s * A_Index
%SplashRef%(Splashy, {vHide: 0, vImgW: spr, vImgH: spr1}*)

} Until (spr > scrWd || spr1 > scrHt)

msgbox

%SplashRef%(Splashy, {vHide: 1, vImgW: "", vImgH: ""}*)
%SplashRef%(Splashy, {vHide: 0}*)






msgbox





;Begin Square


;Dry run to check endpoints

	Loop
	{
	spr := A_Index * ahkWdAdjust + ((A_Index - 1) * ahkWdMod)
	} Until (spr > 20 * scrWd/21)

	if (spr := scrWd - spr)
	startPointWd := floor(spr/2)

spr := 0

	Loop
	{
	spr := A_Index * ahkHtAdjust + (A_Index * ahkHtMod)
	} Until (spr > (20 * scrHt/21) - 2 * ahkHt)

	if (spr := scrHt - spr - 2 * ahkHtMod - ahkHtAdjust)
	startPointHt := floor(spr/2)



spr := 0
Loop
{
spr := startPointHt + A_Index * ahkHtAdjust + (A_Index * ahkHtMod)
AIndex := A_Index
%SplashRef%(Splashy, {mainText: "", subText: "", instance: A_Index, vPosX: 0, vPosY: spr}*)
} Until (spr > (20 * scrHt/21) - 2 * ahkHt)

Loop
{
spr := startPointWd + A_Index * ahkWdAdjust + ((A_Index - 1) * ahkWdMod)
AIndex1 := AIndex + A_Index
%SplashRef%(Splashy, {mainText: "", subText: "", instance: AIndex1, vPosX: spr, vPosY: scrHt - ahkHt}*)
} Until (spr > 20 * scrWd/21)


spr1 := scrWd - ahkWdMod
Loop
{
spr := startPointHt + A_Index * ahkHtAdjust + ((A_Index + 1) * ahkHtMod)
AIndex := AIndex1 + A_Index
%SplashRef%(Splashy, {mainText: "", subText: "", instance: AIndex, vPosX: spr1, vPosY: scrHt - spr}*)
} Until (spr > (20 * scrHt/21) - ahkHt)

Loop
{
spr := startPointWd + A_Index * ahkWdAdjust + (A_Index * ahkWdMod)

AIndex1 := AIndex + A_Index
%SplashRef%(Splashy, {mainText: "", subText: "", instance: AIndex1, vPosX: scrWd - spr, vPosY: 0}*)
} Until (spr > 20 * scrWd/21)
msgbox

Loop %AIndex1%
{
sleep, 50
%SplashRef%(Splashy, {instance: -A_Index}*)
}

msgbox










spr := 0
Loop
{
spr := startPointWd + A_Index * ahkWdAdjust + ((A_Index - 1) * ahkWdMod)

%SplashRef%(Splashy, {mainText: "", subText: "", instance: A_Index, vPosX: spr, vPosY: 0}*)
AIndex := A_Index
} Until (spr > 20 * scrWd/21)

spr1 := scrWd - ahkWdMod
Loop
{
spr := startPointHt + A_Index * ahkHtAdjust + (A_Index * ahkHtMod)
AIndex1 := AIndex + A_Index
%SplashRef%(Splashy, {mainText: "", subText: "", instance: AIndex1, vPosX: spr1, vPosY: spr}*)
} Until (spr > (20 * scrHt/21) - 2 * ahkHt)

Loop
{
spr := startPointWd + A_Index * ahkWdAdjust + (A_Index * ahkWdMod)
AIndex := AIndex1 + A_Index
%SplashRef%(Splashy, {mainText: "", subText: "", instance: AIndex, vPosX: scrWd - spr, vPosY: scrHt - ahkHt}*)
} Until (spr > 20 * scrWd/21)

Loop
{
spr := startPointHt + A_Index * ahkHtAdjust + ((A_Index + 1) * ahkHtMod)
AIndex1 := AIndex + A_Index
%SplashRef%(Splashy, {mainText: "", subText: "", instance: AIndex1, vPosX: 0, vPosY: scrHt - spr}*)
} Until (spr > (20 * scrHt/21) - ahkHt)

msgbox
Loop %AIndex1%
{
sleep, 50
%SplashRef%(Splashy, {instance: -A_Index}*)
}


spr := 0


Loop
{
spr := startPointHt + A_Index * ahkHtAdjust + (A_Index * ahkHtMod)
AIndex := A_Index
%SplashRef%(Splashy, {mainText: "", subText: "", instance: A_Index, vPosX: 0, vPosY: spr}*)
} Until (spr > (20 * scrHt/21) - 2 * ahkHt)

Loop
{
spr := startPointWd + A_Index * ahkWdAdjust + ((A_Index - 1) * ahkWdMod)
AIndex1 := AIndex + A_Index
%SplashRef%(Splashy, {mainText: "", subText: "", instance: AIndex1, vPosX: spr, vPosY: scrHt - ahkHt}*)
} Until (spr > 20 * scrWd/21)

spr1 := scrWd - ahkWdMod
Loop
{
spr := startPointHt + A_Index * ahkHtAdjust + ((A_Index + 1) * ahkHtMod)
AIndex := AIndex1 + A_Index
%SplashRef%(Splashy, {mainText: "", subText: "", instance: AIndex, vPosX: spr1, vPosY: scrHt - spr}*)
} Until (spr > (20 * scrHt/21) - ahkHt)
Loop
{
spr := startPointWd + A_Index * ahkWdAdjust + (A_Index * ahkWdMod)

AIndex1 := AIndex + A_Index
%SplashRef%(Splashy, {mainText: "", subText: "", instance: AIndex1, vPosX: scrWd - spr, vPosY: 0}*)
} Until (spr > 20 * scrWd/21)
msgbox

Loop %AIndex1%
{
sleep, 50
%SplashRef%(Splashy, {instance: -A_Index}*)
}












;Begin Triangle
;ahkWdMod := scrWd/scrHt/2 * (ahkHt) + floor((mod(scrWd/2, ahkHt)/(s/2)))
spr := floor(scrHt/ahkHt)
ahkHtMod := ahkHt + floor(mod(scrHt, ahkHt)/spr)
	if (rWd < scrHt)
	ahkWdMod := rWd/(spr - 1) + floor((mod(rWd, spr)/spr))
	else
	ahkWdMod := rWd/s + floor((mod(rWd, s/2)/(s/2)))

Loop
{
spr := (A_Index - 1) * ahkWdMod
spr1 := (A_Index - 1) * ahkHtMod
;scaleHt := scrWd/scrHt * 

AIndex := A_Index
;msgbox % "rWd " rWd "s " s " spr " spr " triVertLength " triVertLength "`nscrHt - ahkHt " scrHt - ahkHt " spr1 " spr1
%SplashRef%(Splashy, {mainText: "", subText: "", instance: A_Index, vPosX: spr, vPosY: 2 * rHt - spr1}*)
} Until (spr1 > scrHt - 5*ahkHt/4)  ;(spr * spr + spr1 * spr1 > triVertLength * triVertLength)

Loop
{
spr := (AIndex + A_Index - 1) * ahkWdMod
spr1 := (AIndex - A_Index - 1) * ahkHtMod
;msgbox % "spr " spr " spr1 " spr1 " triVertLength " triVertLength
%SplashRef%(Splashy, {mainText: "", subText: "", instance: AIndex + A_Index, vPosX: spr, vPosY: 2 * rHt - spr1}*)		
} Until (spr1 <= 0)
msgbox


Loop %AIndex1%
{
sleep, 50
%SplashRef%(Splashy, {instance: -A_Index}*)
}

Loop
{
spr := (A_Index - 1) * ahkWdMod
spr1 := (A_Index - 1) * ahkHtMod
;scaleHt := scrWd/scrHt * 

AIndex := A_Index
;msgbox % "rWd " rWd "s " s " spr " spr " spr1 " spr1 " triVertLength " triVertLength
%SplashRef%(Splashy, {mainText: "", subText: "", instance: A_Index, vPosX: spr, vPosY: spr1}*)
} Until (spr1 > scrHt - 5*ahkHt/4)

Loop
{
spr := (AIndex + A_Index - 1) * ahkWdMod
spr1 := (AIndex - A_Index - 1) * ahkHtMod
;msgbox % "spr " spr " spr1 " spr1 " triVertLength " triVertLength
%SplashRef%(Splashy, {mainText: "", subText: "", instance: AIndex + A_Index, vPosX: spr, vPosY: spr1}*)		
} Until (spr1 <= 0)

msgbox

Loop %AIndex1%
{
sleep, 50
%SplashRef%(Splashy, {instance: -A_Index}*)
}








Ellipse:




;Begin Ellipse

	Loop, %s%
	{
	sleep 100
		if (A_Index == minIndexBot || (twoBots && A_Index == minIndexBot - 1))
		%SplashRef%(Splashy, {mainText: "bot", instance: A_Index, vPosX: (rWd + rWd * ellXArg[A_Index]), vPosY: (rHt + rHt * ellYArg[A_Index])}*)	
		else
		{
			if (A_Index == minIndexTop || (twoTops && A_Index == minIndexTop - 1))
			%SplashRef%(Splashy, {subText: "top", instance: A_Index, vPosX: (rWd + rWd * ellXArg[A_Index]), vPosY: (rHt + rHt * ellYArg[A_Index])}*)
			else
			%SplashRef%(Splashy, {mainText: "", subText: "", instance: A_Index, vPosX: (rWd + rWd * ellXArg[A_Index]), vPosY: (rHt + rHt * ellYArg[A_Index])}*)		
		}
	}

msgbox % "twoBots " twoBots " ellVertInterval[A_Index] " ellVertInterval[minIndexBot] " ellVertInterval[A_Index - 1] " ellVertInterval[minIndexBot - 1]


	Loop, %s%
	{
	sleep 100
		if (A_Index == minIndexBot || (twoBots && A_Index == minIndexBot - 1))
		{
		%SplashRef%(Splashy, {mainText: "bot", subText: "", instance: A_Index}*)
		;msgbox % "minIndexBot " minIndexBot " minIndexTop " minIndexTop " A_Index " A_Index
		}
		else
		{
			if (A_Index == minIndexTop || (twoTops && A_Index == minIndexTop - 1))
			%SplashRef%(Splashy, {mainText: "", subText: "top", instance: A_Index, vPosX: (rWd + rWd * ellXArg[A_Index]), vPosY: (rHt + rHt * ellYArg[A_Index])}*)
			else
			{
			if Mod(A_Index, 2) is digit
			{
			%SplashRef%(Splashy, {instance: -A_Index}*)
			;msgbox % "minIndexBot " minIndexBot " minIndexTop " minIndexTop " A_Index " A_Index
			}
			}
		}
	}

msgbox elipse reverse

; Ellipse reverse
AIndex := s
While AIndex > 0
{
sleep, 50
%SplashRef%(Splashy, {instance: -AIndex}*)
--AIndex
}

AIndex := s

	While AIndex > 0
	{
	sleep 100

		if (AIndex == minIndexTop || (twoTops && AIndex == minIndexTop - 1))
		%SplashRef%(Splashy, {subText: "top", instance: AIndex, vPosX: (rWd + rWd * ellXArg[AIndex]), vPosY: (rHt + rHt * ellYArg[AIndex])}*)	
		else
		{
			if (AIndex == minIndexBot || (twoBots && AIndex == minIndexBot - 1))
			%SplashRef%(Splashy, {mainText: "bot", instance: AIndex, vPosX: (rWd + rWd * ellXArg[AIndex]), vPosY: (rHt + rHt * ellYArg[AIndex])}*)
			else
			%SplashRef%(Splashy, {mainText: "", subText: "", instance: AIndex, vPosX: (rWd + rWd * ellXArg[AIndex]), vPosY: (rHt + rHt * ellYArg[AIndex])}*)		
		}
;msgbox % "minIndexBot " minIndexBot " minIndexTop " minIndexTop "`nAIndex " AIndex "`nrWd + rWd * ellXArg[AIndex] " rWd + rWd * ellXArg[AIndex]
	--AIndex
	}
msgbox
AIndex := s

	While AIndex > 0
	{
	sleep 100

		if (AIndex == minIndexTop || (twoTops && AIndex == minIndexTop - 1))
		%SplashRef%(Splashy, {subText: "top", mainText: "", instance: AIndex}*)	
		else
		{
			if (AIndex == minIndexBot || (twoBots && AIndex == minIndexBot - 1))
			%SplashRef%(Splashy, {mainText: "bot", subText: "", instance: AIndex, vPosX: (rWd + rWd * ellXArg[AIndex]), vPosY: (rHt + rHt * ellYArg[AIndex])}*)
			else
			{
			if Mod(AIndex, 2) is digit
			%SplashRef%(Splashy, {instance: -AIndex}*)		
			}
		}
	--AIndex
	}

msgbox


;%SplashRef%(Splashy, {instance: 2, imagePath: "", imageUrl: "https://upload.wikimedia.org/wikipedia/commons/thumb/3/36/AutoHotkey_logo.png/220px-AutoHotkey_logo.png"}*)
;%SplashRef%(Splashy, {instance: value, imagePath: pathString, imageUrl: "https://upload.wikimedia.org/wikipedia/commons/thumb/3/36/AutoHotkey_logo.png/220px-AutoHotkey_logo.png"}*)

EnvGet, AutGUI, AutoHotkey
return
Esc::
%SplashRef%(Splashy, {release: 1}*)
ExitApp