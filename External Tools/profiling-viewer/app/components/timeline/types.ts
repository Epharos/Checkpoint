export type Event = {
  name: string;
  threadId: string;
  startTime: number;
  endTime: number;
  startCycle?: number;
  endCycle?: number;
};

export type Placed = Event & { lane: number };

export const defaultPalette = ["#4f46e5", "#10b981", "#f59e0b", "#ef4444", "#06b6d4", "#8b5cf6"];

export function colorFor(seed: string, palette = defaultPalette) {
  const n = Array.from(seed).reduce((a, c) => a + c.charCodeAt(0), 0);
  return palette[n % palette.length];
}

export function formatDuration(start: number, end: number) {
  return (end - start).toFixed(3);
}
