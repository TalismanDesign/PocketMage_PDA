#include <pocketmage_audio/playback_session.h>

#include <cassert>

int main() {
  PlaybackSession session;
  assert(session.id() == 0 && session.result(0) == PlaybackSession::Cancelled);

  // Natural end of an accepted request.
  const uint32_t first = session.request();
  assert(first != 0 && session.result(first) == PlaybackSession::Pending);
  session.start();
  session.finish(true);
  assert(session.result(first) == PlaybackSession::Ended);

  // A newer request supersedes the old ID even if it is rejected.
  const uint32_t rejected = session.request();
  assert(rejected != first && session.result(first) == PlaybackSession::Cancelled);
  session.fail();
  assert(session.result(rejected) == PlaybackSession::Failed);

  // A stale decoder finishing after a rejection cannot turn it into ENDED.
  const uint32_t replaced = session.request();
  session.start();
  const uint32_t latest = session.request();
  session.fail();
  session.finish(true);
  assert(session.result(replaced) == PlaybackSession::Cancelled);
  assert(session.result(latest) == PlaybackSession::Failed);

  // Cancel wins over a later finish, and terminal results do not change.
  const uint32_t stopped = session.request();
  session.start();
  session.cancel();
  session.finish(true);
  assert(session.result(stopped) == PlaybackSession::Cancelled);
  session.fail();
  assert(session.result(stopped) == PlaybackSession::Cancelled);

  // A decoder failure reports FAILED once.
  const uint32_t broken = session.request();
  session.start();
  session.finish(false);
  session.finish(true);
  assert(session.result(broken) == PlaybackSession::Failed);
}
