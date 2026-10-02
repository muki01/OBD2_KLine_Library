# Contributing to the OBD2 K-Line Library

Thank you for taking the time to contribute! Every vehicle test, bug report, fix and idea makes this library better for the whole car-hacking and maker community.

By participating, you agree to follow the [Code of Conduct](CODE_OF_CONDUCT.md).

## Ways to Contribute

| | |
|---|---|
| 🚗 **Report a tested vehicle** | Tell us which cars work (or don't). Use the **Vehicle report** issue template. |
| 🐛 **Report a bug** | Use the **Bug report** template and include the debug output whenever possible. |
| 💡 **Suggest a feature** | Use the **Feature request** template. |
| 🔧 **Submit code** | Protocol fixes, timing improvements, board support, new examples. |
| 📝 **Improve the docs** | Clearer explanations, wiring photos, corrections. |

## Reporting Bugs

Before opening an issue, please search the [existing issues](https://github.com/muki01/OBD2_KLine_Library/issues). A good report includes:

- The library version or commit
- The board (e.g. ESP32-S3, Arduino Nano) and the Arduino core version
- The interface circuit (transistor, LM393, L9637D, MC33290, …)
- The vehicle: make, model, year and engine
- The selected protocol and init type, and the protocol that was detected
- **The debug output** from `setDebug()`. The raw bytes sent and received are the most useful information.

## Development Workflow

1. **Fork** the repository and create a branch:
   ```bash
   git checkout -b feature/my-improvement
   ```
2. Make your changes, keeping them **focused**: one fix or feature per pull request.
3. **Test on a real vehicle** when your change touches the handshake, the framing or the timing, and say in the pull request which vehicle and protocol you tested on.
4. Make sure everything still **compiles** for the boards it supports.
5. Commit with a clear message.
6. Push and open a **pull request**, filling in the template.

## Coding Guidelines

- Follow the existing style of the file you are editing: naming, indentation and comment density.
- Respect the layers: `OBD2_KLine_Core` moves bytes, `KLine_Protocol` builds packets and connects, `KLine_Functions` holds stateless helpers, and everything under `ecus/` is a diagnostic vocabulary.
- Protocol defaults belong in the protocol tables at the top of `KLine_Protocol.cpp`, not in the engine.
- Do not change protocol timings (P1–P4, W1–W4) without testing on a vehicle.
- Wrap constant debug strings in `F()` to save RAM on AVR boards.
- New public methods need an entry in `keywords.txt` and in the API tables of the README.

## ECU-Specific Code

Definitions for specific ECUs are maintained separately and are not part of this repository. Pull requests should target the generic layers and the standard OBD-II vocabulary.

## License

By contributing, you agree that your contributions are licensed under the [GNU General Public License v3.0](LICENSE), and that the author may also offer them under a commercial license.
