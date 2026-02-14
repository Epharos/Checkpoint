import { useEffect, useState } from "react";

export function useView(minTime: number, maxTime: number, containerWidth: number | null, leftLabel = 140, minContentWidth = 600) {
  const contentAvailable = containerWidth && containerWidth > 0 ? Math.max(minContentWidth, containerWidth - 40) : 1000;
  const cWidth = contentAvailable - leftLabel - 20;
  const cSvgWidth = leftLabel + cWidth + 20;

  const [viewStart, setViewStart] = useState(minTime);
  const [viewEnd, setViewEnd] = useState(maxTime);
  useEffect(() => {
    setViewStart(minTime);
    setViewEnd(maxTime);
  }, [minTime, maxTime]);

  const setViewWindow = (newStart: number, newEnd: number) => {
    const min = minTime;
    const max = maxTime;
    const w = Math.max(1e-6, newEnd - newStart);
    let ns = newStart;
    let ne = ns + w;
    if (ns < min) {
      ns = min;
      ne = ns + w;
    }
    if (ne > max) {
      ne = max;
      ns = ne - w;
    }
    setViewStart(ns);
    setViewEnd(ne);
  };

  const viewScale = cWidth / Math.max(1, viewEnd - viewStart);

  const zoomAt = (centerTime: number, factor: number) => {
    const range = viewEnd - viewStart;
    const newRange = range / factor;
    const ns = centerTime - (centerTime - viewStart) / range * newRange;
    const ne = ns + newRange;
    setViewWindow(ns, ne);
  };

  const panBy = (fraction: number) => {
    const range = viewEnd - viewStart;
    const delta = range * fraction;
    setViewWindow(viewStart + delta, viewEnd + delta);
  };

  return { viewStart, viewEnd, setViewWindow, zoomAt, panBy, viewScale, cWidth, cSvgWidth };
}
