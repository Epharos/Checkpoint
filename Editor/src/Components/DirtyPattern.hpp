#pragma once

class DirtyPattern
{
protected:
	bool dirty = true;

public:
	inline constexpr const bool IsDirty() const { return dirty; }

	inline void MarkDirty() { dirty = true; }
	inline void MarkClean() { dirty = false; }
};