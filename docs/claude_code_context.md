# TinyML Workout Form Coach — Project Context

## Class Context
This is a final project for a TinyML class in the UW ECE Professional Master's Program (PMP). The deadline is June 10, 2026 — approximately 6 weeks from now. We are a two-person team with roughly 12–15 hours per person per week available. A 3–5 page project proposal PDF is also required. Hardware reimbursement is available through PMP (contact: May Lim).

We have a Zoom call pending with our professor to clarify two things:
1. Does TinyML require strict MCU-class hardware, or is constrained edge hardware with a dedicated NPU acceptable?
2. If MCU, is MicroPython acceptable for the scripting layer, or is C/TFLite Micro required?

The professor is open to hardware suggestions.

---

## Project Direction
A self-contained, privacy-preserving, vision-based workout assistant. A camera watches someone exercise, identifies what exercise they are doing, counts reps, and scores form quality per rep — all in real time, with no cloud dependency.

### What is not fully decided yet
- **Primary exercises:** Squat and push-up are the current leading candidates, but this is still open. They were chosen because they are biomechanically distinct, high injury-risk if done with bad form, require no equipment, and are visually different enough (vertical vs horizontal movement) to give the classifier distinct signal. Lunge was dropped due to time constraints. We are open to better exercise choices if justified.
- **Form scoring:** The specific form errors to target, and how to define and label them, are still open. Current thinking is geometry-based labels derived from keypoint angles rather than purely subjective human judgment. For example, knee valgus detected when knee_x deviates from ankle_x beyond a threshold. This needs further development.
- **Pipeline approach:** The current idea is described below, but we are open to improvements or alternative approaches if better methods exist.

### What is decided
- Vision-based, not IMU-based. Form errors are geometrically defined and invisible to an accelerometer.
- Self-contained on-device inference. No cloud, no phone required during operation.
- Side-profile camera placement (~1–1.5m from subject, hip height) for best visibility of joint geometry.
- Serial output to laptop for the class demo. No onboard display for the class version — intentionally scoped down.
- Custom self-collected dataset. No existing public dataset has per-rep form quality labels for gym exercises.

---

## Current Pipeline Idea (open to improvement)

```
Camera
  ↓ raw frames
Pose Estimation — MoveNet Lightning (pre-built, not trained by us)
  ↓ 17 keypoints per frame (x, y, confidence) = 51 floats/frame
Windowing — 30-frame sliding ring buffer (~1 second at 30fps)
  ↓ 30 × 51 = 1,530 floats
1D CNN Temporal Classifier — trained by us, deployed via TFLite
  ↓
[Exercise class] [Form score per rep] [Rep count]
  ↓
Serial output to laptop
```

### Key ideas behind this approach
- **MoveNet Lightning** is a pre-built Google pose estimation model that outputs 17 body keypoints (nose, shoulders, elbows, wrists, hips, knees, ankles) as (x, y, confidence) per frame. We use it as-is — we do not train it.
- **Windowing** accumulates keypoint frames into a sliding window. A single frame cannot tell you what exercise is being performed or whether form is good — you need to watch the movement unfold over time.
- **Temporal classifier** reads a sequence of keypoint frames and classifies both exercise identity and form quality. Called "temporal" because it classifies movement over time, not a single snapshot.
- **Multi-task output:** one shared model trunk feeding two heads — exercise class and form score.
- **Training** happens entirely on a laptop in Keras/TensorFlow. The model is never trained on-device. Output is a `.tflite` file deployed to the device.
- **Quantization:** post-training INT8 quantization via TFLite converter. A quantization ablation study (float32 vs INT8 accuracy delta and latency on real hardware) is the primary rigorous experiment of the project.

### What was evaluated and rejected
- **BlazePose:** 33 keypoints vs MoveNet's 17, richer detail — but designed for mobile CPUs, no supported compilation path for target hardware.
- **Transfer learning / UCI HAR / WISDM:** IMU-based datasets, not relevant to a keypoint vision pipeline.

---

## Hardware (not decided — pending professor Zoom call)

### Option A — If MCU strictly required
**OpenMV AE3**
- Alif Ensemble E3 chip: dual Cortex-M55 + dual Ethos-U55 NPUs, 250 GOPS
- 13.5MB on-chip SRAM + 32MB external flash
- Camera included
- ~20fps pose estimation — sufficient for controlled exercise movements
- MicroPython scripting layer, compiled firmware underneath
- Ships from Hong Kong: $90 board + $40 express shipping + ~$15–25 customs = ~$155 total
- True MCU, unambiguous TinyML classification
- OpenMV has a 10-year track record and active ecosystem

### Option B — If edge hardware acceptable
**Raspberry Pi 5 4GB + Hailo-8L AI HAT (13 TOPS) + Camera Module 3**
- RPi 5 4GB: $130
- Hailo-8L AI HAT 13 TOPS: $84
- Camera Module 3: ~$25
- Total: ~$239
- MoveNet Lightning on Hailo NPU at 30fps, 1D CNN classifier on RPi CPU via TFLite
- Ships domestically in 2 days, no customs risk
- Full Linux, best development experience, lowest timeline risk
- Gray area on strict TinyML definition since host runs a full Linux OS

### Dropped hardware and why
| Board | Reason |
|---|---|
| Arduino Nano 33 BLE | No camera, 256KB RAM — cannot support vision pipeline |
| Grove Vision AI V2 | Documented firmware reliability issues, 2.4MB RAM too tight |
| OpenMV H7 Plus | No NPU, CPU-only inference too slow for pose estimation |
| OpenMV N6 | Same shipping risk as AE3, more expensive, AE3 is sufficient |
| Sipeed MaixCAM Pro | Weaker TinyML argument, thinner ecosystem |
| Rockchip RK3588 / Orange Pi 5 | Thin English docs, RKNN toolchain risk on 6-week timeline |
| Jetson Orin Nano | Overkill, over budget |

---

## Future Improvements (post-class V2)
- Onboard display for standalone feedback
- BLE companion smartphone app
- Additional exercises — deadlift is a strong candidate but needs more data and careful labeling given injury risk

---

## What We Need Help With
Use this context to help us continue designing, refining, and eventually implementing this project. The pipeline, exercise selection, form scoring approach, and other design decisions are still open to improvement. If you see a better approach, propose it and explain the tradeoff. Do not lock in decisions that have not been explicitly confirmed.
