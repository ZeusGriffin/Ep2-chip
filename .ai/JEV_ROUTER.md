# JEV ROUTER — BLINK / BY ZEUS

Default decision layer for this repository.

- Model: `typesafe-ai/jev` through Vercel AI Gateway.
- Use Jev for structured decisions only: routing tools/skills, classify intent, continue/retry/ask/stop, risk/urgency scoring, and output verification.
- Do NOT use Jev as the main generative model for coding, design, prose, research, or conversation.
- Keep the main capable model for generation; use Jev before/after it when a fast typed decision saves work.
- Preserve existing project requirements and approved behavior.
- Never commit secrets. Runtime credential: `AI_GATEWAY_API_KEY` in local/Vercel environment only.
- If the gateway/key is unavailable, fall back cleanly to the existing workflow; never block the project solely because Jev is unavailable.

TypeScript pattern:

```ts
import { experimental_evaluate as evaluate } from 'ai';

const decision = await evaluate({
  model: 'typesafe-ai/jev',
  state,
  questions: {
    nextStep: {
      type: 'choice',
      options: ['continue', 'retry', 'ask', 'stop'],
      instructions: 'Choose the safest useful next step.'
    },
    verified: {
      type: 'boolean',
      instructions: 'Does the result satisfy the stated acceptance checks?'
    }
  }
});
```

For all new work: route only when useful, generate with the appropriate main model, verify before delivery.
