"use client";

import React from "react";

export default function OptionsPanel({
  laneHeight,
  setLaneHeight,
  timeUnit,
  setTimeUnit,
  scrollZoom,
  setScrollZoom,
  onClose,
}: {
  laneHeight: number;
  setLaneHeight: (n: number) => void;
  timeUnit: "us" | "ms" | "s" | "raw";
  setTimeUnit: (u: "us" | "ms" | "s" | "raw") => void;
  scrollZoom: boolean;
  setScrollZoom: (b: boolean) => void;
  onClose?: () => void;
}) {
  return (
    <div style={{ position: 'absolute', right: 8, top: 8, zIndex: 50, background: 'white', padding: 12, borderRadius: 8, boxShadow: '0 6px 18px rgba(0,0,0,0.12)' }}>
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: 8 }}>
        <strong>Options</strong>
        {onClose && <button onClick={onClose} style={{ marginLeft: 8 }}>✕</button>}
      </div>

      <div style={{ marginBottom: 8 }}>
        <label style={{ display: 'block', fontSize: 12, marginBottom: 4 }}>Box size</label>
        <input type="range" min={10} max={48} value={laneHeight} onChange={(e) => setLaneHeight(Number(e.target.value))} />
        <div style={{ fontSize: 12 }}>{laneHeight}px</div>
      </div>

      <div style={{ marginBottom: 8 }}>
        <label style={{ display: 'block', fontSize: 12, marginBottom: 4 }}>Time unit</label>
        <select value={timeUnit} onChange={(e) => setTimeUnit(e.target.value as any)}>
          <option value="us">µs</option>
          <option value="ms">ms</option>
          <option value="s">s</option>
          <option value="raw">raw</option>
        </select>
      </div>

      <div style={{ display: 'flex', alignItems: 'center', gap: 8 }}>
        <input id="scrollZoom" type="checkbox" checked={scrollZoom} onChange={(e) => setScrollZoom(e.target.checked)} />
        <label htmlFor="scrollZoom" style={{ fontSize: 12 }}>Scroll to zoom</label>
      </div>
    </div>
  );
}
