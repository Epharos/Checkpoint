import { Event, Placed } from "./types";

export type LaneOptions = {
  laneHeight?: number;
  headerHeight?: number;
};

export function computeLanes(events: Event[], opts: LaneOptions = {}) {
  const laneHeight = opts.laneHeight ?? 18;
  const headerHeight = opts.headerHeight ?? 18;

  const byThread = new Map<string, Event[]>();
  for (const e of events) {
    const arr = byThread.get(e.threadId) ?? [];
    arr.push(e);
    byThread.set(e.threadId, arr);
  }

  const threadOrder = Array.from(byThread.keys()).sort();
  const placedByThread = new Map<string, Placed[]>();
  const perThreadHeights = new Map<string, number>();

  let totalHeight = 0;
  let cursor = 40; // starting offset

  for (const t of threadOrder) {
    const items = (byThread.get(t) || []).slice().sort((a, b) => a.startTime - b.startTime);
    const lanesEnd: number[] = [];
    const placed: Placed[] = [];
    for (const it of items) {
      let lane = lanesEnd.findIndex((end) => end <= it.startTime);
      if (lane === -1) {
        lane = lanesEnd.length;
        lanesEnd.push(it.endTime);
      } else {
        lanesEnd[lane] = it.endTime;
      }
      placed.push({ ...it, lane });
    }
    const threadHeight = Math.max(1, lanesEnd.length) * laneHeight + headerHeight;
    placedByThread.set(t, placed);
    perThreadHeights.set(t, threadHeight);
    totalHeight += threadHeight;
    cursor += threadHeight;
  }

  return { threadOrder, placedByThread, perThreadHeights, totalHeight };
}
