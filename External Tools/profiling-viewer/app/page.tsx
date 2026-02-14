"use client";

import React, { useState } from "react";
import Timeline from "./components/timeline";
import Image from "next/image";

type Event = {
  name: string;
  threadId: string;
  startTime: number;
  endTime: number;
  startCycle?: number;
  endCycle?: number;
};

function parseCSV(text: string): Event[] {
  const lines = text.split(/\r?\n/).map((l) => l.trim()).filter(Boolean);
  if (lines.length === 0) return [];
  const header = lines[0].split(/,|\t/).map((h) => h.trim().toLowerCase());
  const idx = (name: string) => header.findIndex((h) => h.includes(name));
  const nameI = idx("name");
  const threadI = idx("thread");
  const startI = idx("start");
  const endI = idx("end");
  const startC = idx("startcycle");
  const endC = idx("endcycle");

  return lines.slice(1).map((ln) => {
    const cols = ln.split(/,|\t/).map((c) => c.trim());
    const get = (i: number) => (i >= 0 && i < cols.length ? cols[i] : "");
    const sTime = parseFloat(get(startI)) || parseFloat(get(startC)) || 0;
    const eTime = parseFloat(get(endI)) || parseFloat(get(endC)) || 0;
    return {
      name: get(nameI) || cols[0] || "event",
      threadId: get(threadI) || "main",
      startTime: sTime,
      endTime: eTime || sTime + 1,
      startCycle: startC >= 0 ? Number(get(startC)) : undefined,
      endCycle: endC >= 0 ? Number(get(endC)) : undefined,
    };
  });
}

function tryParse(text: string): Event[] {
  // Try JSON first
  try {
    const j = JSON.parse(text);
    if (Array.isArray(j)) {
      return j.map((item: any) => ({
        name: String(item.name ?? item.label ?? "event"),
        threadId: String(item.threadId ?? item.thread ?? "main"),
        startTime: Number(item.startTime ?? item.start ?? item.startCycle ?? 0),
        endTime: Number(item.endTime ?? item.end ?? item.endCycle ?? (item.startTime ? item.startTime + 1 : 1)),
        startCycle: item.startCycle ? Number(item.startCycle) : undefined,
        endCycle: item.endCycle ? Number(item.endCycle) : undefined,
      }));
    }
  } catch (e) {
    // ignore
  }
  // Fallback to CSV
  return parseCSV(text);
}

export default function Home() {
  const [showTimeline, setShowTimeline] = useState(false);
  const [events, setEvents] = useState<Event[]>([]);
  const [raw, setRaw] = useState<string>("");

  const onFile = (f: File | null) => {
    if (!f) return;
    const r = new FileReader();
    r.onload = () => {
      const txt = String(r.result ?? "");
      setRaw(txt);
      const parsed = tryParse(txt);
      setEvents(parsed);
    };
    r.readAsText(f);
  };

  return (
    <main className="p-6 bg-slate-100">

      <header className="flex flex-row align-middle items-center gap-8 mb-8">
        <Image src="/logo.png" width={48} height={48} alt="Checkpoint's logo" />
        <h1 className="text-2xl font-bold">Checkpoint profiler output visualizer</h1>
      </header>

      {!showTimeline ? 
      <div className="flex flex-row align-middle justify-center items-center gap-4 mb-4">
        <div className="mb-4 bg-slate-300 p-4 rounded">
          <p className="font-semibold">Load a profiler output file (CSV, TSV, or JSON)</p>
          <input
            aria-label="Upload profiler output"
            type="file"
            accept=".csv,.tsv,.txt,application/json,text/*"
            onChange={(e) => {
              onFile(e.target.files ? e.target.files[0] : null);
              setShowTimeline(true);
            }}
            className="border p-2 mt-4 w-full bg-slate-200 rounded"
          />
        </div>

        <div className="mb-4">
          <p className="font-semibold">- or -</p>
        </div>

        <div className="mb-4 bg-slate-300 p-4 rounded">
          <p className="font-semibold">Paste the content of the output</p>

          <textarea
            placeholder="Or paste the content here (JSON/CSV)"
            value={raw}
            onChange={(e) => setRaw(e.target.value)}
            rows={8}
            cols={60}
            className="w-full border p-2 bg-slate-200 rounded mt-4 mb-4"
          />

          <button
            onClick={() => {
              const parsed = tryParse(raw);
              setShowTimeline(true);
              setEvents(parsed);
            }}
            className="bg-sky-600 text-white px-3 py-1 rounded mx-auto block"
          >
            Parse
          </button>
        </div>
      </div>
 : (
    <>
      <div className="mb-6">
        <strong>{events.length}</strong> événements parsés.
      </div>

      <div className="w-full h-80 rounded bg-slate-300">
        <Timeline events={events} leftLabel={64} />
      </div>
    </>
 )}

      
    </main>
  );
}

