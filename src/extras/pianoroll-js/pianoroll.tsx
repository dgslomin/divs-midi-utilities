
import React, { useEffect, useRef } from "react";
import * as midifile from "@sreal/midifile";

export function PianoRoll({ midiFile, width, height }: {
	midiFile: midifile.MidiFile,
	width: number,
	height: number
}): React.JSX.Element
{
	const canvasRef = useRef<HTMLCanvasElement | null>(null);

	useEffect(() => {
		const canvas = canvasRef.current!;
		const context = canvas.getContext("2d")!;
	}, []);

	return <canvas ref={canvasRef} width={width} height={height} />;
}