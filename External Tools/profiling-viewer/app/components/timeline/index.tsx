"use client";

import React, { useRef, useState } from "react";
import { Event } from "./types";
import { computeLanes } from "./computeLanes";
import { useView } from "./useView";
import Controls from "./Controls";
import Tooltip from "./Tooltip";
import OptionsPanel from "./OptionsPanel";
import { colorFor, defaultPalette } from "./types";

export default function Timeline({
  events,
  leftLabel = 140,
  minContentWidth = 600,
  threadGap = 12,
  palette = defaultPalette,
}: { events: Event[]; leftLabel?: number; minContentWidth?: number; threadGap?: number; palette?: string[];}) {
  const containerRef = useRef<HTMLDivElement | null>(null);
  const [containerWidth, setContainerWidth] = useState<number | null>(null);
  React.useEffect(() => {
    const el = containerRef.current;
    if (!el) return;
    const ro = new ResizeObserver(() => setContainerWidth(Math.max(0, el.clientWidth)));
    ro.observe(el);
    setContainerWidth(Math.max(0, el.clientWidth));
    return () => ro.disconnect();
  }, []);

  const [laneHeight, setLaneHeight] = useState<number>(18);
  const [timeUnit, setTimeUnit] = useState<"us" | "ms" | "s" | "raw">("us");
  const [scrollZoomEnabled, setScrollZoomEnabled] = useState<boolean>(true);
  const [showOptions, setShowOptions] = useState<boolean>(false);

  const { threadOrder, placedByThread, perThreadHeights, totalHeight } = React.useMemo(() => computeLanes(events, { laneHeight }), [events, laneHeight]);

  const minTime = React.useMemo(() => Math.min(...events.map((e) => e.startTime)), [events]);
  const maxTime = React.useMemo(() => Math.max(...events.map((e) => e.endTime)), [events]);

  const { viewStart, viewEnd, zoomAt, panBy, viewScale, cWidth, cSvgWidth } = useView(minTime, maxTime, containerWidth, leftLabel, minContentWidth);

  const threadOffsets = React.useMemo(() => {
    const map = new Map<string, number>();
    let cursor = 40;
    for (const t of threadOrder) {
      map.set(t, cursor);
      cursor += (perThreadHeights.get(t) || 36) + threadGap;
    }
    return map;
  }, [threadOrder, perThreadHeights, threadGap]);

  const totalHeightWithGaps = React.useMemo(() => {
    const gaps = Math.max(0, threadOrder.length - 1) * threadGap;
    return totalHeight + gaps;
  }, [totalHeight, threadOrder.length, threadGap]);

  const svgRef = useRef<SVGSVGElement | null>(null);
  const [hover, setHover] = useState<{ ev: any | null; clientX: number; clientY: number }>({ ev: null, clientX: 0, clientY: 0 });

  const showHover = (ev: any | null, clientX = 0, clientY = 0) => setHover({ ev, clientX, clientY });

  const [isDragging, setIsDragging] = useState(false);
  const lastXRef = useRef<number | null>(null);

  const onPointerDown = (e: React.PointerEvent<SVGSVGElement>) => {
    const svg = svgRef.current;
    if (!svg) return;
    try {
      (e.currentTarget as Element).setPointerCapture(e.pointerId);
    } catch (err) {}
    setIsDragging(true);
    lastXRef.current = e.clientX;
  };

  const onPointerMove = (e: React.PointerEvent<SVGSVGElement>) => {
    if (!isDragging) return;
    if (lastXRef.current === null) return;
    const dx = e.clientX - lastXRef.current;
    if (Math.abs(dx) < 1) return;
    lastXRef.current = e.clientX;
    if (cWidth > 0) {
      panBy(-dx / cWidth);
    }
  };

  const onPointerUp = (e: React.PointerEvent<SVGSVGElement>) => {
    try {
      (e.currentTarget as Element).releasePointerCapture(e.pointerId);
    } catch (err) {}
    setIsDragging(false);
    lastXRef.current = null;
  };

  const resetView = () => {
    zoomAt((viewStart + viewEnd) / 2, 1 / 1.0);
  };

  const zoomIn = () => zoomAt((viewStart + viewEnd) / 2, 1.5);
  const zoomOut = () => zoomAt((viewStart + viewEnd) / 2, 1 / 1.5);

  return (
    <div ref={containerRef} style={{ overflowX: "auto", position: "relative" }}>
      <Controls onZoomIn={zoomIn} onZoomOut={zoomOut} onReset={resetView} />
      <button onClick={() => setShowOptions((v) => !v)} style={{ position: 'absolute', left: 160, top: 15, zIndex: 50 }}>
        Options
      </button>
      {showOptions ? <OptionsPanel laneHeight={laneHeight} setLaneHeight={setLaneHeight} timeUnit={timeUnit} setTimeUnit={setTimeUnit} scrollZoom={scrollZoomEnabled} setScrollZoom={setScrollZoomEnabled} /> : null}
      <svg
        ref={svgRef}
        width={cSvgWidth}
        height={totalHeightWithGaps + 80}
        onPointerDown={onPointerDown}
        onPointerMove={onPointerMove}
        onPointerUp={onPointerUp}
        onPointerCancel={onPointerUp}
        style={{ touchAction: 'none', cursor: isDragging ? 'grabbing' : 'grab' }}
      >
        {threadOrder.map((t) => {
          const base = threadOffsets.get(t) || 0;
          return (
            <g key={t}>
              <text x={leftLabel - 10} y={base + 12} fontSize={12} textAnchor="end" fill="#111">
                {t}
              </text>
              <line x1={leftLabel} x2={cSvgWidth - 10} y1={base + 8} y2={base + 8} stroke="#eee" />
            </g>
          );
        })}

        {threadOrder.flatMap((t) => (placedByThread.get(t) || []).map((ev, idx) => ({ t, ev, idx }))).map(({ t, ev, idx }) => {
          
          

          const base = threadOffsets.get(t) || 0;
          const laneH = laneHeight;
          const gap = 4;
          const y = base + 22 + ev.lane * (laneH + gap);
          const x = leftLabel + (ev.startTime - viewStart) * viewScale;
          const w = Math.max(2, (ev.endTime - ev.startTime) * viewScale);
          const color = colorFor(ev.name + ev.threadId + ev.lane, palette);

          return (
  
            <g key={`${t}-${idx}`}> 
              <rect
                x={x}
                y={y}
                width={w}
                height={laneH}
                rx={3}
                fill={color}
                opacity={0.95}
                style={{ cursor: 'pointer' }}
                onMouseEnter={(e) => showHover(ev, e.clientX, e.clientY)}
                onMouseMove={(e) => showHover(ev, e.clientX, e.clientY)}
                onMouseLeave={() => showHover(null, 0, 0)}
              />
              <text x={x + 6} y={y + 12} fontSize={10} fill="#fff" pointerEvents="none">
                {ev.name}
              </text>
            </g>
          );
        })}

        <g>
          <line x1={leftLabel} x2={cSvgWidth - 10} y1={totalHeightWithGaps + 60} y2={totalHeightWithGaps + 60} stroke="#000" />
          {[0, 0.25, 0.5, 0.75, 1].map((p) => {
            const t = viewStart + p * (viewEnd - viewStart);
            const x = leftLabel + (t - viewStart) * viewScale;
            return (
              <g key={p}>
                <line x1={x} x2={x} y1={totalHeight + 68} y2={totalHeight + 76} stroke="#000" />
                <text x={x} y={totalHeightWithGaps + 78} fontSize={11} textAnchor="middle">
                  {formatTime(t - minTime, timeUnit)}
                </text>
              </g>
            );
          })}
        </g>
      </svg>
      
      <Tooltip ev={hover.ev} clientX={hover.clientX} clientY={hover.clientY} minTime={minTime} timeUnit={timeUnit} />
    </div>
  );
}

function formatTime(value: number, unit: "us" | "ms" | "s" | "raw") {
  switch (unit) {
    case "raw":
      return String(Math.round(value));
    case "s":
      return `${(value / 1_000_000).toFixed(3)} s`;
    case "ms":
      return `${(value / 1000).toFixed(3)} ms`;
    case "us":
    default:
      return `${Math.round(value).toString()} µs`;
  }
}
