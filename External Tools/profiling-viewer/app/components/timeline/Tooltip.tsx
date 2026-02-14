"use client";

import React from "react";
import { createPortal } from "react-dom";
import { Placed } from "./types";

export default function Tooltip({ ev, clientX, clientY, minTime, timeUnit = "us" }: { ev: Placed | null; clientX: number; clientY: number; minTime: number; timeUnit?: "us" | "ms" | "s" | "raw" }) {
  if (!ev || typeof document === "undefined") return null;
  const left = Math.min(window.innerWidth - 240, clientX + 12);
  const top = Math.max(8, clientY + 12);
  return createPortal(
    <div
      style={{
        position: 'fixed',
        left,
        top,
        background: 'rgba(0,0,0,0.9)',
        color: '#fff',
        padding: '8px 10px',
        borderRadius: 6,
        fontSize: 12,
        pointerEvents: 'none',
        width: 220,
        zIndex: 9999
      }}
    >
      <div style={{ fontWeight: 600, marginBottom: 6 }}>{ev.name}</div>
      <div>Thread: {ev.threadId}</div>
      <div>Start: {formatTime(ev.startTime - minTime, timeUnit)} — End: {formatTime(ev.endTime - minTime, timeUnit)}</div>
      <div>Duration: {formatTime(ev.endTime - ev.startTime, timeUnit)}</div>
      {typeof ev.startCycle === 'number' && typeof ev.endCycle === 'number' && (
        <>
          <div>Cycles: {ev.endCycle - ev.startCycle}</div>
          <div>Cycles/{formatTimeUnit(timeUnit)}: {((timeUnitOrder(timeUnit) * (ev.endCycle - ev.startCycle) / Math.max(1e-9, (ev.endTime - ev.startTime))).toFixed(3))} per {formatTimeUnit(timeUnit)}</div>
        </>
      )}
    </div>, document.body
  );
}

function timeUnitOrder(unit: "us" | "ms" | "s" | "raw") {
  switch (unit) {
    case "us": return 1;
    case "ms": return 1_000;
    case "s": return 1_000_000;
    case "raw": return 1;
  }
}

function formatTimeUnit(unit: "us" | "ms" | "s" | "raw")
{
    if(unit === "us") return "µs";
    return unit;
}

function formatTime(value: number, unit: "us" | "ms" | "s" | "raw") {
  switch (unit) {
    case "raw":
      return String(Math.round(value));
    case "s":
      return `${(value / 1_000_000).toFixed(6)} s`;
    case "ms":
      return `${(value / 1000).toFixed(3)} ms`;
    case "us":
    default:
      return `${Math.round(value).toString()} µs`;
  }
}
