#include "gameplay/survival_audio.hpp"

#include "scene_data.hpp"
#include "scripts/script.hpp"
#include "sound_bank.gen.hpp"

namespace Deep_forest {
namespace {

constexpr int kSfxChannelsPerBus = 16;
int channelPriority[kSfxChannelsPerBus] = {};
int priorityBus = -1;
int nextChannel = 0;
unsigned char missingSampleLogged[SND_COUNT] = {};

int soundIndex(SurvivalSound sound) {
  return static_cast<int>(sound);
}

int soundPriority(SurvivalSound sound) {
  switch (sound) {
    case SurvivalSound::Gunshot:
      return 5;
    case SurvivalSound::Reload:
      return 4;
    case SurvivalSound::ZombieAttack:
      return 3;
    case SurvivalSound::EmptyShot:
      return 2;
    case SurvivalSound::Footstep:
      return 1;
  }
  return 0;
}

int chooseChannel(ScriptContext& ctx, int priority) {
  const int bus = ctx.reverbBusBase;
  if (priorityBus != bus) {
    for (int i = 0; i < kSfxChannelsPerBus; ++i) channelPriority[i] = 0;
    priorityBus = bus;
  }

  const int core = bus == 0 ? 1 : 0;
  const u32 ended = ctx.engine->audio.adpcm.endedMask(core);
  for (int offset = 0; offset < kSfxChannelsPerBus; ++offset) {
    const int channel = (nextChannel + offset) % kSfxChannelsPerBus;
    if ((ended & (1u << channel)) == 0) continue;
    channelPriority[channel] = priority;
    nextChannel = (channel + 1) % kSfxChannelsPerBus;
    return bus + channel;
  }

  int victim = -1;
  int lowestPriority = priority;
  for (int channel = 0; channel < kSfxChannelsPerBus; ++channel) {
    if (channelPriority[channel] >= lowestPriority) continue;
    lowestPriority = channelPriority[channel];
    victim = channel;
  }
  if (victim < 0) return -1;
  channelPriority[victim] = priority;
  return bus + victim;
}

}  // namespace

void playSurvivalSound(ScriptContext& ctx, SurvivalSound sound) {
  if (!ctx.engine || ctx.sfxVolume <= 0) return;

  const int index = soundIndex(sound);
  if (index < 0 || index >= SND_COUNT) {
    TYRA_ERROR("Survival audio: sound index ", index,
               " is missing from the project sound list");
    return;
  }

  audsrv_adpcm_t* sample = soundbank::sample(index);
  if (!sample) {
    if (!missingSampleLogged[index]) {
      TYRA_WARN("Survival audio: sample failed to load: ", SND_PATHS[index]);
      missingSampleLogged[index] = 1;
    }
    return;
  }

  const int channel = chooseChannel(ctx, soundPriority(sound));
  if (channel < 0) return;

  ctx.engine->audio.adpcm.setVolume(
      static_cast<u8>(ctx.sfxVolume > 100 ? 100 : ctx.sfxVolume),
      static_cast<s8>(channel));
  ctx.engine->audio.reverb.setChannelSend(static_cast<s8>(channel), true);
  const Tyra::AdpcmResult result =
      ctx.engine->audio.adpcm.forcePlay(sample, static_cast<s8>(channel));
  if (result != Tyra::ADPCM_OK)
    TYRA_WARN("Survival audio: playback failed for ", SND_PATHS[index],
              " (result ", static_cast<int>(result), ")");
}

}  // namespace Deep_forest
