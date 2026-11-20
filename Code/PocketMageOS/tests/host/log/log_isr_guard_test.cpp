#include <pocketmage_log/log_core.h>

#include <cassert>

int main() {
  using pm_log::LogContext;

  assert(!pm_log::lineFromWriterRequest(false, LogContext::Task, true, false));
  assert(pm_log::lineFromWriterRequest(true, LogContext::Task, true, false));
  assert(pm_log::lineFromWriterRequest(true, LogContext::Task, false, true));
  assert(!pm_log::lineFromWriterRequest(true, LogContext::Isr, true, false));
  assert(!pm_log::lineFromWriterRequest(true, LogContext::Isr, false, true));
}
