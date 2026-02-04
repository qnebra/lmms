// AutomationClip.h

// Mutex to make methods involving automation clips thread safe

#include <QMutex>

...

// Unlocked Helper Methods
// Existing methods from lines 219-228 remain unchanged

...

mutable QRecursiveMutex m_clipMutex;

// Rest of the contents of the file remain as they were, except for the above modifications.