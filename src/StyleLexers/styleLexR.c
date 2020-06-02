#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_R = {
// Language Keywords
"Inf NA NA_character_ NA_complex_ NA_integer_ NA_real_ NULL NaN break else false for function if in next "
"repeat true while",
// Base / Default package function
"abbreviate abline abs acf acos acosh addmargins aggregate agrep alarm alias alist all anova any aov aperm "
"append apply approx approxfun apropos ar args arima array arrows asin asinh assign assocplot atan atanh "
"attach attr attributes autoload autoloader ave axis backsolve barplot basename beta bindtextdomain "
"binomial biplot bitmap bmp body box boxplot bquote break browser builtins bxp by bzfile c call cancor "
"capabilities casefold cat category cbind ccf ceiling character charmatch chartr chol choose chull "
"citation class close cm cmdscale codes coef coefficients col colnames colors colorspaces colours comment "
"complex confint conflicts contour contrasts contributors convolve cophenetic coplot cor cos cosh cov "
"covratio cpgram crossprod cummax cummin cumprod cumsum curve cut cutree cycle data dataentry date dbeta "
"dbinom dcauchy dchisq de debug debugger decompose delay deltat demo dendrapply density deparse deriv det "
"detach determinant deviance dexp df dfbeta dfbetas dffits dgamma dgeom dget dhyper diag diff diffinv "
"difftime digamma dim dimnames dir dirname dist dlnorm dlogis dmultinom dnbinom dnorm dotchart double "
"dpois dput drop dsignrank dt dump dunif duplicated dweibull dwilcox eapply ecdf edit effects eigen emacs "
"embed end environment eval evalq example exists exp expression factanal factor factorial family fft fifo "
"file filter find fitted fivenum fix floor flush for force formals format formula forwardsolve "
"fourfoldplot frame frequency ftable function gamma gaussian gc gcinfo gctorture get getenv geterrmessage "
"gettext gettextf getwd gl glm globalenv gray grep grey grid gsub gzcon gzfile hat hatvalues hcl hclust "
"head heatmap help hist history hsv httpclient iconv iconvlist identical identify if ifelse image "
"influence inherits integer integrate interaction interactive intersect invisible isoreg jitter jpeg "
"julian kappa kernapply kernel kmeans knots kronecker ksmooth labels lag lapply layout lbeta lchoose lcm "
"legend length letters levels lfactorial lgamma library licence license line lines list lm load "
"loadhistory loadings local locator loess log logb logical loglin lowess ls lsfit machine mad mahalanobis "
"makepredictcall manova mapply match matlines matplot matpoints matrix max mean median medpolish menu "
"merge message methods mget min missing mode monthplot months mosaicplot mtext mvfft names napredict "
"naprint naresid nargs nchar ncol next nextn ngettext nlevels nlm nls noquote nrow numeric objects offset "
"open optim optimise optimize options order ordered outer pacf page pairlist pairs palette par parse paste "
"pbeta pbinom pbirthday pcauchy pchisq pdf pentagamma person persp pexp pf pgamma pgeom phyper pi pico "
"pictex pie piechart pipe plclust plnorm plogis plot pmatch pmax pmin pnbinom png pnorm points poisson "
"poly polygon polym polyroot postscript power ppoints ppois ppr prcomp predict preplot pretty princomp "
"print prmatrix prod profile profiler proj promax prompt provide psigamma psignrank pt ptukey punif "
"pweibull pwilcox q qbeta qbinom qbirthday qcauchy qchisq qexp qf qgamma qgeom qhyper qlnorm qlogis "
"qnbinom qnorm qpois qqline qqnorm qqplot qr qsignrank qt qtukey quantile quarters quasi quasibinomial "
"quasipoisson quit qunif quote qweibull qwilcox rainbow range rank raw rbeta rbind rbinom rcauchy rchisq "
"readline real recover rect reformulate regexpr relevel remove reorder rep repeat replace replicate "
"replications require reshape resid residuals restart return rev rexp rf rgamma rgb rgeom rhyper rle "
"rlnorm rlogis rm rmultinom rnbinom rnorm round row rownames rowsum rpois rsignrank rstandard rstudent rt "
"rug runif runmed rweibull rwilcox sample sapply save savehistory scale scan screen screeplot sd search "
"searchpaths seek segments seq sequence serialize setdiff setequal setwd shell sign signif sin single sinh "
"sink smooth solve sort source spectrum spline splinefun split sprintf sqrt stack stars start stderr stdin "
"stdout stem step stepfun stl stop stopifnot str strftime strheight stripchart strptime strsplit strtrim "
"structure strwidth strwrap sub subset substitute substr substring sum summary sunflowerplot supsmu svd "
"sweep switch symbols symnum system t table tabulate tail tan tanh tapply tempdir tempfile termplot terms "
"tetragamma text time title toeplitz tolower topenv toupper trace traceback transform trigamma trunc "
"truncate try ts tsdiag tsp typeof unclass undebug union unique uniroot unix unlink unlist unname "
"unserialize unsplit unstack untrace unz update upgrade url var varimax vcov vector version vi vignette "
"warning warnings weekdays weights which while window windows with write wsbrowser xedit xemacs xfig xinch "
"xor xtabs xyinch yinch zapsmall",
// "Other Package Functions
"acme aids aircondit amis aml banking barchart barley beaver bigcity boot brambles breslow bs bwplot calcium "
"cane capability cav censboot channing city claridge cloth cloud coal condense contourplot control corr "
"darwin densityplot dogs dotplot ducks empinf envelope environmental ethanol fir frets gpar grav gravity "
"grob hirose histogram islay knn larrows levelplot llines logit lpoints lsegments lset ltext lvqinit "
"lvqtest manaus melanoma melanoma motor multiedit neuro nitrofen nodal ns nuclear oneway parallel paulsen "
"poisons polar qq qqmath remission rfs saddle salinity shingle simplex singer somgrid splom stripplot "
"survival tau tmd tsboot tuna unit urine viewport wireframe wool xyplot",
// Unused
"",
// Unused
"",
// ---
NULL,
};


EDITLEXER lexR = { 
SCLEX_R, IDS_LEX_R_STAT, L"R-S-SPlus Statistics Code", L"r; rdata; rds; rda", L"", 
&KeyWords_R,{
    { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //{ {SCE_R_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { {SCE_R_COMMENT}, IDS_LEX_STR_63127, L"Comment", L"fore:#008000", L"" },
    { {SCE_R_KWORD}, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#0A246A", L"" },
    { {SCE_R_BASEKWORD}, IDS_LEX_STR_63271, L"Base Package Functions", L"bold; fore:#7F0000", L"" },
    { {SCE_R_OTHERKWORD}, IDS_LEX_STR_63272, L"Other Package Functions", L"bold; fore:#7F007F", L"" },
    { {SCE_R_NUMBER}, IDS_LEX_STR_63130, L"Number", L"fore:#0000FF", L"" },
    { {MULTI_STYLE(SCE_R_STRING,SCE_R_STRING2,0,0)}, IDS_LEX_STR_63131, L"String", L"italic; fore:#3C6CDD", L"" },
    { {SCE_R_OPERATOR}, IDS_LEX_STR_63132, L"Operator", L"bold; fore:#B000B0", L"" },
    { {SCE_R_IDENTIFIER}, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
    { {SCE_R_INFIX}, IDS_LEX_STR_63269, L"Infix", L"fore:#660066", L"" },
    { {SCE_R_INFIXEOL}, IDS_LEX_STR_63270, L"Infix EOL", L"fore:#FF4000; back:#E0C0E0; eolfilled", L"" },
    EDITLEXER_SENTINEL } };
