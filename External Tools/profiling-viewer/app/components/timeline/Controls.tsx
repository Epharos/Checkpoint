"use client";

import React from "react";

export default function Controls({ onZoomIn, onZoomOut, onReset, onToggleOptions }: { onZoomIn: () => void; onZoomOut: () => void; onReset: () => void; onToggleOptions?: () => void; }) {
  return (
    <div style={{ position: "absolute", left: 8, top: 8, zIndex: 10, display: "flex", gap: 8 }}>
      <button onClick={onZoomIn} title="Zoom in" style={{ padding: 6 }}>+</button>
      <button onClick={onZoomOut} title="Zoom out" style={{ padding: 6 }}>−</button>
      <button onClick={onReset} title="Reset view" style={{ padding: 6 }}>Reset</button>
      {onToggleOptions && (
        <button onClick={onToggleOptions} title="Options" style={{ padding: 6 }}>Options</button>
      )}
    </div>
  );
}
